#include "distance_fsm.h"

/* Standard library includes */
#include <stdbool.h>

/* User library includes */
#include "ddl/distance/distance_types.h"
#include "ddl/ddl_config.h"
#include "util/log/log.h"
#include "osal/osal.h"

typedef enum eFrameFields
{
    eFRAME_HEADER_SYNC   = 0x57,
    eFRAME_REQUEST_MARK  = 0x10,
    eFRAME_RESPONSE_MARK = 0x00,
    eFRAME_RESERVED      = 0xFFFF,
    eFRAME_ID            = 0x00
} eFrameFields;

typedef struct 
{
    uint8_t  header;
    uint8_t  mark;
    uint8_t  reserved;
    uint8_t  id;
    uint32_t system_time;
    uint8_t  dis_1000[3];
    uint8_t  dis_status;
    uint16_t signal_strength;
    uint8_t  range_percision;
    uint8_t  checksum;
} TOFSenseFrame;

typedef struct __attribute__((packed))
{
    uint8_t  header;
    uint8_t  mark;
    uint16_t reserved0;
    uint8_t  id;
    uint16_t reserved1;
    uint8_t  checksum;
} TOFSenseReadCmd;

static TOFSenseFrame resp_frame;

static const TOFSenseReadCmd read_cmd = {
    .header    = eFRAME_HEADER_SYNC,
    .mark      = eFRAME_REQUEST_MARK,
    .reserved0 = eFRAME_RESERVED,
    .id        = eFRAME_ID,
    .reserved1 = eFRAME_RESERVED,
    .checksum  = 0x63
};

static uint8_t checksum(const TOFSenseFrame *frame)
{
    const uint8_t* buffer = (const uint8_t*)frame;
    const uint32_t length = sizeof(TOFSenseFrame) - 1;
    uint8_t        sum    = 0;

    for(uint32_t i = 0; i < length; i++)
    {
        sum += buffer[i];
    }

    return sum;
}

static uint32_t to_little_endian32(uint32_t value)
{
    uint8_t *buf = (uint8_t *)&value;

    return ((uint32_t)buf[0] << 0)  |
           ((uint32_t)buf[1] << 8)  |
           ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[3] << 24);
}

static uint16_t to_little_endian16(uint16_t value)
{
    uint8_t *buf = (uint8_t *)&value;

    return (uint16_t)(((uint16_t)buf[0] << 0) |
                      ((uint16_t)buf[1] << 8));
}

static bool is_frame_valid(const TOFSenseFrame *frame, uint32_t old_system_time)
{
    if(frame->header != eFRAME_HEADER_SYNC)
    {
        LOG_DEBUG("HEADER: Expected 0x%02X, got 0x%02X", eFRAME_HEADER_SYNC, frame->header);
        return false;
    }

    if(frame->mark != eFRAME_RESPONSE_MARK)
    {
        LOG_DEBUG("MARK: Expected 0x%02X, got 0x%02X", eFRAME_RESPONSE_MARK, frame->mark);
        return false;
    }

    if(frame->id != eFRAME_ID)
    {
        LOG_DEBUG("ID: Expected 0x%02X, got 0x%02X", eFRAME_ID, frame->id);
        return false;
    }

    if(to_little_endian32(frame->system_time) == old_system_time)
    {
        LOG_DEBUG("System Time stuck: 0x%08X", frame->system_time);
        return false;
    }

    if(frame->checksum != checksum(frame))
    {
        LOG_DEBUG("Checksum: Expected 0x%02X, got 0x%02X", checksum(frame), frame->checksum);
        return false;
    }

    return true;
}

static void update_distance_frame(DistanceObject* aobj, const TOFSenseFrame *frame)
{
    /* Update The validity frame */
    aobj->frame->valid = true;

    /* Calculate the distance in meters */
    aobj->frame->distance = (float)(
        (uint32_t)(
            ((uint32_t)0)                        |
            ((uint32_t)frame->dis_1000[0] << 8)  | 
            ((uint32_t)frame->dis_1000[1] << 16) | 
            ((uint32_t)frame->dis_1000[2] << 24) 
        ) / 256
    ) / 1000.0f;

    /* Single byte values, endianness doesnt matter */
    aobj->frame->status    = frame->dis_status;
    aobj->frame->precision = frame->range_percision;
    aobj->frame->strength  = to_little_endian16(frame->signal_strength);

    /* Update last sensor time */
    aobj->system_time = to_little_endian32(frame->system_time);

    /* Print sanity check */
    LOG_DEBUG("distance: %f", (double)aobj->frame->distance);
    LOG_DEBUG("status: %u", aobj->frame->status);
    LOG_DEBUG("precision: %u", aobj->frame->precision);
    LOG_DEBUG("strength: %u", aobj->frame->strength);
    LOG_DEBUG("system_time: %u", aobj->system_time);
}

static void retry_handler(DistanceObject* aobj, FSM* fsm)
{
    aobj->retry++;
    if(aobj->retry < eDISTANCE_READ_RETRY_MAX)
    {
        (void)util_fsm_transition(fsm, distance_read_state);
    }
    else
    {
        LOG_DEBUG("Retries exceeded limit (%u)", aobj->retry);
        aobj->frame->valid = false;
        (void)util_fsm_transition(fsm, distance_idle_state);
    }
}

static void timeout_handler(void* arg)
{
    static Event timeout_event = { .type = eDISTANCE_EVENT_TIMEOUT };
    DistanceObject* aobj = (DistanceObject*)arg;
    (void)util_active_object_post(&aobj->aobj, &timeout_event);
}

#if 0
static void uart_rx_complete_handler(void* arg)
{
    static Event frame_received_event = { .type = eDISTANCE_EVENT_FRAME_RECEIVED };
    DistanceObject* aobj = (DistanceObject*)arg;
    (void)util_active_object_post(&aobj->aobj, &frame_received_event);
}
#endif

void distance_init_state(FSM* fsm, Event* event)
{
    DistanceObject* aobj = (DistanceObject*)fsm->arg;
    switch(event->type)
    {
    case eFSM_EVENT_INIT:
        LOG_DEBUG("Sensor init called");
        if(osal_timer_init(&aobj->timer, timeout_handler, aobj))
        {
            (void)util_fsm_transition(fsm, distance_error_state);
        }
        else
        {
            (void)util_fsm_transition(fsm, distance_idle_state);
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("Sensor init finished");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void distance_error_state(FSM* fsm, Event* event)
{
    (void)fsm;
    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_ERROR("Non recoverable error");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void distance_idle_state(FSM* fsm, Event* event)
{
    DistanceObject* aobj = (DistanceObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("IDLE entered");
        aobj->retry = 0;
        break;
    case eDISTANCE_EVENT_READ:
        LOG_DEBUG("Attempt to read a frame from sensor");
        (void)util_fsm_transition(fsm, distance_read_state);
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("IDLE exited");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void distance_read_state(FSM* fsm, Event* event)
{
    DistanceObject* aobj = (DistanceObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("READ entered");
        (void)read_cmd;
        // TODO: send the read_cmd
        // TODO: Prepare reading into resp_frame
        (void)osal_timer_arm(aobj->timer, eDISTANCE_READ_TIMEOUT_MS, eTIMER_TYPE_ONCE);
        break;
    case eDISTANCE_EVENT_FRAME_RECEIVED:
        LOG_DEBUG("Frame Received!");
        (void)util_fsm_transition(fsm, distance_update_state);
        break;
    case eDISTANCE_EVENT_TIMEOUT:
        LOG_DEBUG("Read timed out");
        // TODO: Abort current RX reception
        retry_handler(aobj, fsm);
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("READ exited");
        (void)osal_timer_disarm(aobj->timer);
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void distance_update_state(FSM* fsm, Event* event)
{
    DistanceObject* aobj = (DistanceObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("UPDATE entered");
        if(is_frame_valid(&resp_frame, aobj->system_time))
        {
            LOG_DEBUG("Frame is valid");
            update_distance_frame(aobj, &resp_frame);
            (void)util_fsm_transition(fsm, distance_idle_state);
        }
        else
        {
            LOG_DEBUG("Frame is invalid");
            retry_handler(aobj, fsm);
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("UPDATE exited");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}