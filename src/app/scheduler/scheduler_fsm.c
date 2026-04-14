#include "scheduler_fsm.h"

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "app/scheduler/scheduler_config.h"
#include "app/scheduler/scheduler_types.h"
#include "util/log/log.h"
#include "osal/osal.h"

static TimerArg timer_arg;

static void tick_handler(void* arg)
{
    static Event tick_event = { .type = eSCHEDULER_EVENT_TICK };
    SchedulerObject* aobj = (SchedulerObject*)arg;
    // Post eSCHEDULER_EVENT_TICK to itself 
    (void)util_active_object_post(&aobj->aobj, &tick_event);
}

void scheduler_init_state(FSM* fsm, Event* event)
{
    SchedulerObject* aobj = (SchedulerObject*)fsm->arg;
    switch(event->type)
    {
    case eFSM_EVENT_INIT:
        LOG_DEBUG("INIT entry");
        timer_arg.handler = tick_handler;
        timer_arg.arg = aobj;
        if(osal_timer_init(&aobj->timer, &timer_arg))
        {
            (void)util_fsm_transition(fsm, scheduler_error_state);
        }
        else
        {
            (void)util_fsm_transition(fsm, scheduler_idle_state);
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("INIT exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void scheduler_error_state(FSM* fsm, Event* event)
{
    (void)fsm;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_ERROR("ERROR entry");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void scheduler_idle_state(FSM* fsm, Event* event)
{
    SchedulerObject* aobj = (SchedulerObject*)fsm->arg;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("IDLE entry");
        aobj->tick = 0;
        break;
    case eSCHEDULER_EVENT_START:
        LOG_DEBUG("Start event received");
        (void)util_fsm_transition(fsm, scheduler_run_state);
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("IDLE exit");
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}

void scheduler_run_state(FSM* fsm, Event* event)
{
    SchedulerObject* aobj = (SchedulerObject*)fsm->arg;
    eStatus status;

    switch(event->type)
    {
    case eFSM_EVENT_ENTRY:
        LOG_DEBUG("IDLE entry");
        (void)osal_timer_arm(aobj->timer, eSCHEDULER_TICK_MS, eTIMER_TYPE_REPEAT);
        status = aobj->subscribers[0].module_post(aobj->subscribers[0].module, aobj->subscribers[0].event);
        if(status)
        {
            LOG_ERROR("Scheduler failed to alert registered subscriber at slot 0");
        }
        break;
    case eSCHEDULER_EVENT_STOP:
        LOG_DEBUG("Scheduler stop event received");
        (void)util_fsm_transition(fsm, scheduler_idle_state);
        break;
    case eSCHEDULER_EVENT_TICK:
        aobj->tick = (aobj->tick + 1) % eSCHEDULER_SUBSCRIBERS_MAX;
        if(aobj->subscribers[aobj->tick].module_post)
        {
            status = aobj->subscribers[aobj->tick].module_post(aobj->subscribers[aobj->tick].module, aobj->subscribers[aobj->tick].event);
            if(status)
            {
                LOG_ERROR("Scheduler failed to alert registered subscriber at slot %u", aobj->tick);
            }
        }
        break;
    case eFSM_EVENT_EXIT:
        LOG_DEBUG("IDLE exit");
        (void)osal_timer_disarm(aobj->timer);
        break;
    default:
        LOG_WARNING("Unknown event type %u", event->type);
    }
}