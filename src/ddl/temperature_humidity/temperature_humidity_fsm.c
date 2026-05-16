#include "temperature_humidity_fsm.h"  /* you'll create this header alongside; see note at end */

/* Standard library includes */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* User library includes */
#include "ddl/temperature_humidity/temperature_humidity_config.h"
#include "ddl/temperature_humidity/temperature_humidity_types.h"
#include "hal/gpio/hal_gpio_config.h"
#include "hal/gpio/hal_gpio.h"
#include "util/log/log.h"
#include "osal/osal.h"

typedef struct
{
    uint8_t humidity_hi;
    uint8_t humidity_lo;
    uint8_t temperature_hi;
    uint8_t temperature_lo;
    uint8_t parity;
} TempHumFrame;

static TempHumFrame resp_frame;
static uint64_t last_read_tick;

static uint64_t am2302_get_time_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
}

/* Poll the line until it reaches target_level or until timeout_us elapses.
 * On success, writes the timestamp at which the level was first observed
 * into *out_timestamp_us (NULL allowed). Used to distinguish low bits from
 * high bits when reading data. */
static eStatus am2302_wait_for_level(int target_level, uint64_t timeout_us,
                                     uint64_t* out_timestamp_us)
{
    uint64_t deadline = am2302_get_time_us() + timeout_us;
    int level;

    while(true)
    {
        eStatus status = hal_gpio_read(eTEMPERATURE_HUMIDITY_GPIO_DEVICE, &level);
        if(status != eSTATUS_SUCCESSFUL)
        {
            return eSTATUS_DEVICE_ERROR;
        }

        uint64_t now = am2302_get_time_us();
        if(level == target_level)
        {
            if(out_timestamp_us != NULL)
            {
                *out_timestamp_us = now;
            }
            return eSTATUS_SUCCESSFUL;
        }
        if(now >= deadline)
        {
            return eSTATUS_ACTION_FAILED;
        }
    }
}

/* Drive the line low for AM2302_START_SIGNAL_HOLD_MS, then release.
 * Leaves the line as INPUT on return. */
static eStatus am2302_send_read_request(void)
{
    eStatus status;

    /* Drive low. set_direction(OUTPUT) initialises the driven value to 0,
     * so this single call is the start of the low pulse. */
    status = hal_gpio_set_direction(eTEMPERATURE_HUMIDITY_GPIO_DEVICE, eGPIO_OUTPUT);
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }

    osal_delay_ms(AM2302_START_SIGNAL_HOLD_MS);

    /* Release the bus. The external pull-up pulls the line back high
     * within a few microseconds; the sensor then takes over within Tgo. */
    return hal_gpio_set_direction(eTEMPERATURE_HUMIDITY_GPIO_DEVICE, eGPIO_INPUT);
}

/* Wait through the sensor's 80 µs low + 80 µs high response.
 * On return the line is low (start of bit-0's 50 µs preamble). */
static eStatus am2302_wait_for_response(void)
{
    eStatus status;

    /* After we released the bus, line should go LOW within Tgo (20..200 µs)
     * as the sensor starts its response. */
    status = am2302_wait_for_level(0, AM2302_RESPONSE_TIMEOUT_US, NULL);
    if(status)
    {
        LOG_WARNING("Temperature-Humidity sensor did not respond to start signal");
        return status;
    }

    /* Wait through the 80 µs low response. */
    status = am2302_wait_for_level(1, AM2302_RESPONSE_PHASE_TIMEOUT_US, NULL);
    if(status != eSTATUS_SUCCESSFUL)
    {
        LOG_WARNING("Temperature-Humidity sensor response-low phase did not end");
        return status;
    }

    /* Wait through the 80 µs high response. */
    status = am2302_wait_for_level(0, AM2302_RESPONSE_PHASE_TIMEOUT_US, NULL);
    if(status != eSTATUS_SUCCESSFUL)
    {
        LOG_WARNING("Temperature-Humidity sensor response-high phase did not end");
        return status;
    }

    return eSTATUS_SUCCESSFUL;
}

/* Read one data bit. Caller's line state on entry: low (in a 50 µs preamble).
 * Bit value is decided by how long the following high pulse lasts. */
static eStatus am2302_read_bit(uint8_t* out_bit)
{
    uint64_t t_rise, t_fall;
    eStatus  status;

    /* Wait for the line to go high (end of 50 µs preamble). */
    status = am2302_wait_for_level(1, AM2302_BIT_TIMEOUT_US, &t_rise);
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }

    /* Wait for the line to go low (end of this bit's high pulse). */
    status = am2302_wait_for_level(0, AM2302_BIT_TIMEOUT_US, &t_fall);
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }

    *out_bit = ((t_fall - t_rise) > AM2302_BIT_THRESHOLD_US) ? 1u : 0u;
    return eSTATUS_SUCCESSFUL;
}

static eStatus am2302_read_frame(TempHumFrame* frame)
{
    uint8_t* frame_bytes = (uint8_t*)frame;

    for(uint32_t i = 0; i < AM2302_BYTE_COUNT; i++)
    {
        frame_bytes[i] = 0;
    }

    for(uint32_t i = 0; i < AM2302_BIT_COUNT; i++)
    {
        uint8_t bit;
        eStatus status = am2302_read_bit(&bit);
        if(status != eSTATUS_SUCCESSFUL)
        {
            LOG_WARNING("AM2302 bit %u read failed", i);
            return status;
        }
        frame_bytes[i / 8] = (uint8_t)((frame_bytes[i / 8] << 1) | bit);
    }
    return eSTATUS_SUCCESSFUL;
}

static bool am2302_parity_check(const TempHumFrame* frame)
{
    /* parity = (humidity_hi + humidity_lo + temp_hi + temp_lo)'s low 8 bits. */
    uint8_t sum = (uint8_t)(frame->humidity_hi + frame->humidity_lo +
                            frame->temperature_hi + frame->temperature_lo);
    return sum == frame->parity;
}

static void am2302_decode_frame(TemperatureHumidityObject* aobj, const TempHumFrame* frame)
{
    /* Humidity: 16-bit unsigned, value is 10× the actual %RH. */
    uint16_t raw_humidity = (uint16_t)(((uint16_t)frame->humidity_hi << 8) | 
                                        frame->humidity_lo);
    aobj->frame->humidity = (float)raw_humidity / 10.0f;

    /* Temperature: sign-magnitude (NOT two's complement). Bit 15 = sign,
     * bits 14..0 = magnitude in 0.1 °C units. In the datasheet under
     * "Special Instructions" we see -10.1 °C is encoded as 0x8065, not 0xFF9B. */
    uint16_t raw_temp  = (uint16_t)(((uint16_t)frame->temperature_hi << 8) | 
                                        frame->temperature_lo);
    bool negative = (raw_temp & 0x8000u) != 0u;
    uint16_t magnitude = raw_temp & 0x7FFFu;
    float temp_c = (float)magnitude / 10.0f;
    aobj->frame->temperature = negative ? -temp_c : temp_c;
}

/* Place the line in its idle state. Call once at module start-up.
 * NOTE: the AM2302 needs ~2 s after power-on before it accepts the first
 * read (datasheet §7.4 step 1). The FSM should respect this — either by
 * being initialised early enough that 2 s elapse before the first read,
 * or by arming a 2 s timer before transitioning to a read state. */
static eStatus am2302_init(void)
{
    eStatus status = hal_gpio_set_direction(eTEMPERATURE_HUMIDITY_GPIO_DEVICE, eGPIO_INPUT);
    if(status == eSTATUS_SUCCESSFUL)
    {
        last_read_tick = am2302_get_time_us();
    }
    return status;
}

/* Perform one read transaction and return decoded values.
 * Caller is responsible for spacing reads at least AM2302_MIN_READ_INTERVAL_MS
 * apart. Returns eSTATUS_ACTION_FAILED on protocol timeout/checksum,
 * eSTATUS_DEVICE_ERROR on HAL failure. Retry on the caller's side. */
static eStatus am2302_read(TemperatureHumidityObject* aobj)
{
    if(aobj == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    eStatus status = am2302_send_read_request();
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }

    status = am2302_wait_for_response();
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }
    
    status = am2302_read_frame(&resp_frame);
    if(status != eSTATUS_SUCCESSFUL)
    {
        return status;
    }

    if(!am2302_parity_check(&resp_frame))
    {
        LOG_WARNING("AM2302 checksum mismatch: %02X+%02X+%02X+%02X != %02X",
                    resp_frame.humidity_hi, resp_frame.humidity_lo,
                    resp_frame.temperature_hi, resp_frame.temperature_lo,
                    resp_frame.parity);
        return eSTATUS_ACTION_FAILED;
    }

    am2302_decode_frame(aobj, &resp_frame);
    LOG_DEBUG("AM2302 read OK: humidity=%.1f%%RH temperature=%.1fC",
              (double)aobj->frame->humidity, (double)aobj->frame->temperature);
    return eSTATUS_SUCCESSFUL;
}

void temperature_humidity_init_state(FSM* fsm, Event* event)
{
    TemperatureHumidityObject* aobj = (TemperatureHumidityObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_INIT:
        LOG_DEBUG("INIT entry");
        aobj->frame->valid = false;
        if(am2302_init())
        {
            (void)util_fsm_transition(fsm, temperature_humidity_error_state);
        }
        else
        {
            (void)util_fsm_transition(fsm, temperature_humidity_idle_state);
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("INIT exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void temperature_humidity_error_state(FSM* fsm, Event* event)
{
    TemperatureHumidityObject* aobj = (TemperatureHumidityObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("ERROR entry");
        aobj->frame->valid = false;
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void temperature_humidity_idle_state(FSM* fsm, Event* event)
{
    TemperatureHumidityObject* aobj = (TemperatureHumidityObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("IDLE entry");
        aobj->retry = 0;
        break;
    case eTEMPERATURE_HUMIDITY_EVENT_READ:
        LOG_DEBUG("Read event received");
        (void)util_fsm_transition(fsm, temperature_humidity_read_state);
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("IDLE exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void temperature_humidity_read_state(FSM* fsm, Event* event)
{
    TemperatureHumidityObject* aobj = (TemperatureHumidityObject*)fsm->arg;
    eStatus status = 0;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("READ entry");
        /* The sensor requires at least 2-seconds interval between reads. */
        if(am2302_get_time_us() - last_read_tick > AM2302_MIN_READ_INTERVAL_MS * 1000ull)
        {
            do
            {
                status = am2302_read(aobj);
                if(status)
                {
                    aobj->retry++;
                }
                else
                {
                    aobj->frame->valid = true;
                    last_read_tick = am2302_get_time_us();
                }
            }while(status && aobj->retry < eTEMPERATURE_HUMIDITY_RETRY_MAX);

            if(status)
            {
                LOG_DEBUG("Retries exhausted (%u)", aobj->retry);
                aobj->frame->valid = false;
            }
        }

        /* There is no transition in IDLE entry case, so there shouldn't
         * be a risk for stack overflow. */
        (void)util_fsm_transition(fsm, temperature_humidity_idle_state);

        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("READ exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}