#ifndef UTIL_FSM_H
#define UTIL_FSM_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "status.h"

typedef enum eFSMEvent
{
    eFSM_EVENT_INIT,
    eFSM_EVENT_ENTRY,
    eFSM_EVENT_EXIT,
    eFSM_EVENT_USER
} eFSMEvent;

typedef struct Event Event;
typedef struct FSM FSM;

typedef void (*StateFP)(FSM* fsm, Event* event);

struct Event
{
    uint32_t type;
};

struct FSM
{
    StateFP current_state;
    void*   arg;
};

/**
 * @brief   Initialize an FSM.
 * @details Initializes a Finite-State Machine to be used with Active Object.
 * @param   fsm A pointer to an uninitialized FSM struct.
 * @param   init_state A function pointer to the machine's initial state.
 * @param   arg An argument (if required - can be of any type).
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      fsm or init_state are NULL
 */
eStatus util_fsm_init(FSM* fsm, StateFP init_state, void* arg);

/**
 * @brief   Send an event to an FSM.
 * @param   fsm A pointer to an initialized FSM struct.
 * @param   event An event from @ref eFSMEvent (to be expanded for practical use).
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      fsm is NULL or uninitialized
 */
eStatus util_fsm_send_event(FSM* fsm, Event* event);

/**
 * @brief   Handles transition between two states.
 * @param   fsm A pointer to an initialized FSM struct.
 * @param   next_state A function pointer to the machine's next state.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      next_state or fsm are NULL or fsm is uninitialized
 */
eStatus util_fsm_transition(FSM* fsm, StateFP next_state);

#endif