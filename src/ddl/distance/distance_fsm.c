#include "distance_fsm.h"

/* Standard library includes */
#include <stdbool.h>
#include <stdint.h>

/* User library includes */
#include "ddl/distance/distance_config.h"
#include "ddl/distance/distance_types.h"
#include "hal/uart/hal_uart.h"
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

static TimerArg timer_arg;

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
        return false;
    }

    if(frame->mark != eFRAME_RESPONSE_MARK)
    {
        return false;
    }

    if(frame->id != eFRAME_ID)
    {
        return false;
    }

    if(to_little_endian32(frame->system_time) == old_system_time)
    {
        return false;
    }

    if(frame->checksum != checksum(frame))
    {
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

static void uart_rx_complete_handler(void* arg)
{
    static Event frame_received_event = { .type = eDISTANCE_EVENT_FRAME_RECEIVED };
    DistanceObject* aobj = (DistanceObject*)arg;
    (void)util_active_object_post(&aobj->aobj, &frame_received_event);
}

void distance_init_state(FSM* fsm, Event* event)
{
    DistanceObject* aobj = (DistanceObject*)fsm->arg;
    switch(event->type)
    {
    case eFSM_EVENT_INIT:
        LOG_DEBUG("INIT entry");
        timer_arg.handler = timeout_handler;
        timer_arg.arg     = aobj;
        if(osal_timer_init(&aobj->timer, &timer_arg))
        {
            (void)util_fsm_transition(fsm, distance_error_state);
        }
        else
        {
            (void)util_fsm_transition(fsm, distance_idle_state);
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("INIT exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void distance_error_state(FSM* fsm, Event* event)
{
    DistanceObject* aobj = (DistanceObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_ERROR("ERROR entry");
        aobj->frame->valid = false;
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
        LOG_DEBUG("IDLE entry");
        aobj->retry = 0;
        break;
    case eDISTANCE_EVENT_READ:
        LOG_DEBUG("Read event received");
        (void)util_fsm_transition(fsm, distance_read_state);
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("IDLE exit");
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
        LOG_DEBUG("READ entry");
        (void)hal_uart_write(eDISTANCE_UART_DEVICE, &read_cmd, sizeof(read_cmd), NULL, NULL);
        (void)hal_uart_read(eDISTANCE_UART_DEVICE, &resp_frame, sizeof(resp_frame), uart_rx_complete_handler, aobj);
        (void)osal_timer_arm(aobj->timer, eDISTANCE_READ_TIMEOUT_MS, eTIMER_TYPE_ONCE);
        break;
    case eDISTANCE_EVENT_FRAME_RECEIVED:
        LOG_DEBUG("Frame Received!");
        (void)util_fsm_transition(fsm, distance_update_state);
        break;
    case eDISTANCE_EVENT_TIMEOUT:
        LOG_DEBUG("Read timed out");
        (void)hal_uart_abort(eDISTANCE_UART_DEVICE);
        retry_handler(aobj, fsm);
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("READ exit");
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
        LOG_DEBUG("UPDATE entry");
        if(is_frame_valid(&resp_frame, aobj->system_time))
        {
            update_distance_frame(aobj, &resp_frame);
            LOG_DEBUG("Frame is valid. Distance measured: %fm", (double)aobj->frame->distance);
            (void)util_fsm_transition(fsm, distance_idle_state);
        }
        else
        {
            LOG_WARNING("Frame is invalid");
            retry_handler(aobj, fsm);
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("UPDATE exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}