#include "fsm.h"

/* Standard library includes */
#include <stddef.h>

eStatus util_fsm_init(FSM* fsm, StateFP init_state, void* arg)
{
    if(fsm == NULL || init_state == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    /* Assign the fsm object values */
    fsm->current_state = init_state;
    fsm->arg           = arg;

    /* Calls the init states with an init event */
    Event init_event = { .type = eFSM_EVENT_INIT };
    fsm->current_state(fsm, &init_event);

    return eSTATUS_SUCCESSFUL;
}

eStatus util_fsm_send_event(FSM* fsm, Event* event)
{
    if(fsm == NULL || fsm->current_state == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    /* Calls the current state with the provided event and data */
    fsm->current_state(fsm, event);

    return eSTATUS_SUCCESSFUL;
}

eStatus util_fsm_transition(FSM* fsm, StateFP next_state)
{
    if(fsm == NULL || fsm->current_state == NULL || next_state == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    /* Calls current state with exit event, sets the next state and calls it
       with an entry event */
    Event exit_event = { .type = eFSM_EVENT_EXIT };
    Event entry_event = { .type = eFSM_EVENT_ENTRY };
    fsm->current_state(fsm, &exit_event);
    fsm->current_state = next_state;
    fsm->current_state(fsm, &entry_event);

    return eSTATUS_SUCCESSFUL;
}