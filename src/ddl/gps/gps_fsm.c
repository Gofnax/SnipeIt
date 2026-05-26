#include "gps_fsm.h"

/* Standard library includes */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* User library includes */
#include "ddl/gps/gps_config.h"
#include "ddl/gps/gps_types.h"
#include "hal/uart/hal_uart.h"
#include "util/log/log.h"
#include "osal/osal.h"

/* UBX frame layout (u-blox proprietary binary protocol).
 *
 *   B5 62 | <class> | <id> | <length lo> <length hi> | <payload...> | <CK_A> <CK_B>
 *
 * The two-byte length field is the PAYLOAD length only — it excludes
 * the 6-byte header and the 2-byte checksum. Multi-byte fields are
 * little-endian on the wire. The Fletcher-style 8-bit checksum runs
 * over (class + id + length + payload), so the two sync bytes are
 * deliberately skipped. */
typedef enum eUbxFrameFields
{
    eUBX_SYNC_1          = 0xB5,
    eUBX_SYNC_2          = 0x62,

    /* NAV class */
    eUBX_CLS_NAV         = 0x01,
    eUBX_ID_NAV_PVT      = 0x07,
    eUBX_PVT_PAYLOAD_LEN = 92,

    /* CFG class */
    eUBX_CLS_CFG         = 0x06,
    eUBX_ID_CFG_MSG      = 0x01,
    eUBX_ID_CFG_RATE     = 0x08,

    /* NMEA standard message class (used as the msgClass field of
     * UBX-CFG-MSG when disabling NMEA streams). */
    eUBX_NMEA_CLASS      = 0xF0,
    eUBX_NMEA_GGA        = 0x00,
    eUBX_NMEA_GLL        = 0x01,
    eUBX_NMEA_GSA        = 0x02,
    eUBX_NMEA_GSV        = 0x03,
    eUBX_NMEA_RMC        = 0x04,
    eUBX_NMEA_VTG        = 0x05,
    eUBX_NMEA_TXT        = 0x41
} eUbxFrameFields;

/* Full UBX-NAV-PVT response frame, including header and checksum.
 * Total size on the wire is exactly 100 bytes. The struct is packed
 * so we can read it directly into a UART buffer and index fields by
 * offset without worrying about padding being inserted between, say,
 * the uint8_t fix_type and the uint8_t flags that follow it. */
typedef struct __attribute__((packed))
{
    /* UBX header (6 bytes) */
    uint8_t  sync1;
    uint8_t  sync2;
    uint8_t  msg_class;
    uint8_t  msg_id;
    uint16_t payload_length;

    /* UBX-NAV-PVT payload (92 bytes) */
    uint32_t i_tow;        /* GPS time of week of nav epoch [ms] */
    uint16_t year;         /* UTC year */
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint8_t  valid_flags;  /* bit0=date, bit1=time, bit2=fully resolved, bit3=mag */
    uint32_t t_acc;        /* time accuracy estimate [ns] */
    int32_t  nano;         /* time accuracy estimate: fraction of second [ns], -1e9..+1e9 */
    uint8_t  fix_type;     /* Quality of the fix: 0=no fix, 1=DR, 2=2D, 3=3D, 4=GNSS+DR, 5=time-only */
    uint8_t  flags;
    uint8_t  flags2;
    uint8_t  num_sv;       /* number of satellites used in solution */
    int32_t  lon;          /* longitude [deg * 1e-7] */
    int32_t  lat;          /* latitude  [deg * 1e-7] */
    int32_t  height;       /* height above ellipsoid [mm] */
    int32_t  height_msl;   /* height above mean sea level [mm] */
    uint32_t h_acc;        /* horizontal accuracy estimate [mm] */
    uint32_t v_acc;        /* vertical accuracy estimate [mm] */
    int32_t  vel_n;        /* NED north velocity [mm/s] */
    int32_t  vel_e;        /* NED east  velocity [mm/s] */
    int32_t  vel_d;        /* NED down  velocity [mm/s] */
    int32_t  g_speed;      /* ground speed (2-D) [mm/s] */
    int32_t  head_mot;     /* heading of motion (2-D) [deg * 1e-5] */
    uint32_t s_acc;        /* speed accuracy estimate [mm/s] */
    uint32_t head_acc;     /* heading accuracy estimate [deg * 1e-5] */
    uint16_t p_dop;        /* position DOP * 0.01 */
    uint8_t  flags3;
    uint8_t  reserved[5];
    int32_t  head_veh;     /* heading of vehicle (2-D) [deg * 1e-5] */
    int16_t  mag_dec;      /* magnetic declination [deg * 1e-2] */
    uint16_t mag_acc;      /* declination accuracy [deg * 1e-2] */

    /* UBX trailer (2 bytes) */
    uint8_t  checksum_a;
    uint8_t  checksum_b;
} UbxNavPvtFrame;

/* UBX-NAV-PVT poll request. Same class/ID as the response, but with
 * zero payload length. The receiver replies with the full NAV-PVT
 * frame above. Total size on the wire is 8 bytes. */
typedef struct __attribute__((packed))
{
    uint8_t  sync1;
    uint8_t  sync2;
    uint8_t  msg_class;
    uint8_t  msg_id;
    uint16_t payload_length;
    uint8_t  checksum_a;
    uint8_t  checksum_b;
} UbxPollCmd;

/* One step in the boot-time configuration sequence. The driver builds
 * a UBX frame from these fields into config_tx_buf, computes the
 * checksum, and issues the UART write. Payloads we send are all
 * small; 16 bytes covers everything we use. */
typedef struct
{
    uint8_t msg_class;
    uint8_t msg_id;
    uint8_t payload_length;
    uint8_t payload[16];
} ConfigStep;

static UbxNavPvtFrame resp_frame;
static uint32_t       last_i_tow;

/* Scratch TX buffer for the configuration walk. Sized for the largest
 * frame we send: UBX-CFG-RATE = 6 header + 6 payload + 2 checksum =
 * 14 bytes. We round up for headroom. */
static uint8_t  config_tx_buf[24];

/* Bookkeeping for the configuration sequence. config_step advances each
 * time the previous write completes; config_user_cb is the user's
 * "configuration done" callback. */
static uint32_t config_step;
static async_cb config_user_cb;
static void*    config_user_arg;

/* Pre-computed poll: checksum below is the Fletcher-8 of
 * (msg_class=0x01, msg_id=0x07, len_lo=0x00, len_hi=0x00). */
static const UbxPollCmd poll_cmd = {
    .sync1          = eUBX_SYNC_1,
    .sync2          = eUBX_SYNC_2,
    .msg_class      = eUBX_CLS_NAV,
    .msg_id         = eUBX_ID_NAV_PVT,
    .payload_length = 0,
    .checksum_a     = 0x08,
    .checksum_b     = 0x19
};

/* Boot-time configuration sequence. Walked in order by
 * gps_configure_for_query_mode -> config_send_step -> config_step_complete.
 *
 * Steps 0..6: silence the seven default NMEA streams on this UART by
 *             sending UBX-CFG-MSG with the short form payload
 *             [msgClass, msgId, rate]; rate=0 disables emission.
 * Step  7  : set the measurement / navigation rate via UBX-CFG-RATE.
 *             The payload is [measRate(2), navRate(2), timeRef(2)],
 *             all little-endian. measRate = 0x03E8 = 1000 ms (1 Hz);
 *             navRate = 1; timeRef = 1 (GPS time).
 *
 * To run at a different rate, change the first two bytes of the
 * CFG-RATE payload: e.g. 250 ms -> 0xFA 0x00 for 4 Hz. The NEO-M8N
 * tops out at 5 Hz with concurrent GPS+GLONASS (10 Hz on a single
 * constellation). Polling faster than the rate just returns stale
 * frames, which gps_is_response_valid() rejects via the iTOW check.
 *
 * The settings are written to RAM only, so they are re-applied on
 * every boot — this avoids flash wear and works even when the on-
 * receiver flash save would not stick. */
static const ConfigStep config_sequence[] = {
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_GGA, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_GLL, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_GSA, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_GSV, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_RMC, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_VTG, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_MSG, 3, { eUBX_NMEA_CLASS, eUBX_NMEA_TXT, 0x00 } },
    { eUBX_CLS_CFG, eUBX_ID_CFG_RATE, 6,
      { 0xE8, 0x03,   /* measRate = 1000 ms (1 Hz)            */
        0x01, 0x00,   /* navRate  = 1 measurement per nav epoch */
        0x01, 0x00 }  /* timeRef  = GPS                         */
    }
};
#define CFG_SEQ_LEN (sizeof(config_sequence) / sizeof(config_sequence[0]))

/* Forward declarations for the configuration-chain internals. */
static void config_send_step(void);
static void config_step_complete(void *arg);

/* === Low-level helpers ============================================ */

/* UBX 8-bit Fletcher checksum over `len` bytes starting at `buf`.
 * Both CK_A and CK_B are returned by reference. */
static void ubx_checksum(const uint8_t *buf, uint32_t len,
                         uint8_t *out_ck_a, uint8_t *out_ck_b)
{
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;

    for(uint32_t i = 0; i < len; i++)
    {
        ck_a = (uint8_t)(ck_a + buf[i]);
        ck_b = (uint8_t)(ck_b + ck_a);
    }

    *out_ck_a = ck_a;
    *out_ck_b = ck_b;
}

static uint16_t to_little_endian16(uint16_t value)
{
    uint8_t *buf = (uint8_t *)&value;

    return (uint16_t)(((uint16_t)buf[0] << 0) |
                      ((uint16_t)buf[1] << 8));
}

static uint32_t to_little_endian32(uint32_t value)
{
    uint8_t *buf = (uint8_t *)&value;

    return ((uint32_t)buf[0] << 0)  |
           ((uint32_t)buf[1] << 8)  |
           ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[3] << 24);
}

static int32_t to_little_endian_i32(int32_t value)
{
    return (int32_t)to_little_endian32((uint32_t)value);
}

static bool is_frame_valid(const UbxNavPvtFrame *frame, uint32_t old_i_tow)
{
    uint8_t expected_checksum_a;
    uint8_t expected_checksum_b;

    if(frame->sync1 != eUBX_SYNC_1 || frame->sync2 != eUBX_SYNC_2)
    {
        return false;
    }

    if(frame->msg_class != eUBX_CLS_NAV || frame->msg_id != eUBX_ID_NAV_PVT)
    {
        return false;
    }

    if(to_little_endian16(frame->payload_length) != eUBX_PVT_PAYLOAD_LEN)
    {
        return false;
    }

    /* Stale-response guard: iTOW advances every nav epoch. If the
     * receiver returns the same iTOW we already saw, the solution
     * hasn't been recomputed yet — treat it as a bad read. The first
     * call will always pass this check because last_i_tow=0 is
     * essentially never seen at run time. */
    if(to_little_endian32(frame->i_tow) == old_i_tow)
    {
        return false;
    }

    /* Checksum covers everything between the sync bytes and the two
     * checksum bytes: cls + id + length + payload = 96 bytes. */
    ubx_checksum(((const uint8_t *)frame) + 2,
                 sizeof(UbxNavPvtFrame) - 4,
                 &expected_checksum_a, &expected_checksum_b);
    if(frame->checksum_a != expected_checksum_a || frame->checksum_b != expected_checksum_b)
    {
        return false;
    }

    return true;
}

static void extract_gps_frame(GPSFrame *out, const UbxNavPvtFrame *frame)
{
    /* Mark the frame valid for consumers */
    out->valid = true;

    /* Position. Lat/lon are reported in 1e-7 degrees (so an int32_t
     * spans roughly ±214°, plenty of headroom). Heights are in mm.
     * We expose meters and degrees to make downstream code easier. */
    out->latitude = (double)to_little_endian_i32(frame->lat) * 1e-7;
    out->longitude = (double)to_little_endian_i32(frame->lon) * 1e-7;
    out->altitude = (float)to_little_endian_i32(frame->height_msl) / 1000.0f;

    /* Quality */
    out->fix_type = frame->fix_type;
    out->num_satellites = frame->num_sv;
    out->h_acc = (float)to_little_endian32(frame->h_acc) / 1000.0f;
}

/* Build the UBX frame for config_sequence[config_step] into config_tx_buf and
 * fire it off. When config_step is past the end of the sequence, invoke
 * the user's "configuration done" callback instead. */
static void config_send_step(void)
{
    const ConfigStep*   step;
    uint32_t            frame_len;
    uint8_t             ck_a;
    uint8_t             ck_b;

    if(config_step >= CFG_SEQ_LEN)
    {
        LOG_DEBUG("Configuration sequence complete");
        if(config_user_cb != NULL)
        {
            config_user_cb(config_user_arg);
        }
        return;
    }

    step = &config_sequence[config_step];

    config_tx_buf[0] = eUBX_SYNC_1;
    config_tx_buf[1] = eUBX_SYNC_2;
    config_tx_buf[2] = step->msg_class;
    config_tx_buf[3] = step->msg_id;
    config_tx_buf[4] = step->payload_length;  /* length lo (payload < 256) */
    config_tx_buf[5] = 0x00;               /* length hi                 */
    for(uint8_t i = 0; i < step->payload_length; i++)
    {
        config_tx_buf[6 + i] = step->payload[i];
    }

    /* Checksum spans cls + id + length(2) + payload. */
    ubx_checksum(&config_tx_buf[2],
                 (uint32_t)(4 + step->payload_length),
                 &ck_a, &ck_b);
    config_tx_buf[6 + step->payload_length]     = ck_a;
    config_tx_buf[6 + step->payload_length + 1] = ck_b;

    frame_len = (uint32_t)(6 + step->payload_length + 2);
    LOG_DEBUG("Sending config step %u (%u bytes)", config_step, frame_len);
    (void)hal_uart_write(eGPS_UART_DEVICE, config_tx_buf, frame_len,
                         config_step_complete, NULL);
}

/* Called by the UART layer when the current configuration write has
 * been pushed out. Advances to the next step. The chain ends when
 * config_send_step sees config_step == CFG_SEQ_LEN. */
static void config_step_complete(void *arg)
{
    (void)arg;
    config_step++;
    config_send_step();
}

/* === Public API =================================================== */

void gps_configure_for_query_mode(async_cb on_complete, void *arg)
{
    LOG_DEBUG("Starting GPS configuration sequence");
    config_user_cb  = on_complete;
    config_user_arg = arg;
    config_step     = 0;
    config_send_step();
}

eStatus gps_request_reading(async_cb on_received, void *arg)
{
    eStatus status;

    /* Fire-and-forget the 8-byte poll. The matching read below is
     * what gets us the 100-byte NAV-PVT response — io-uring serialises
     * submissions to the same fd, so the write goes out first. */
    status = hal_uart_write(eGPS_UART_DEVICE, &poll_cmd, sizeof(poll_cmd),
                            NULL, NULL);
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }

    return hal_uart_read(eGPS_UART_DEVICE, &resp_frame, sizeof(resp_frame),
                         on_received, arg);
}

bool gps_is_response_valid(void)
{
    return is_frame_valid(&resp_frame, last_i_tow);
}

void gps_get_data(GPSFrame *out)
{
    if(out == NULL)
    {
        return;
    }

    extract_gps_frame(out, &resp_frame);

    /* Remember this reading's iTOW so the next call to
     * gps_is_response_valid() can detect a stale repeat. We update
     * here rather than in is_frame_valid() so a rejected/retried
     * frame doesn't poison the stale-check baseline. */
    last_i_tow = to_little_endian32(resp_frame.i_tow);
}

void gps_reset_state(void)
{
    last_i_tow = 0;
}
