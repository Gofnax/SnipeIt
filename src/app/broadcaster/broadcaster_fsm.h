#ifndef APP_BROADCASTER_FSM_H
#define APP_BROADCASTER_FSM_H

/* User library includes */
#include "util/fsm/fsm.h"

/**
 * @brief   Initial state. Transitions to broadcaster_idle_state.
 */
void broadcaster_init_state(FSM* fsm, Event* event);

/**
 * @brief   Idle state. Handles eBROADCASTER_EVENT_UPDATE by copying the live
 *          DDLFrame into the broadcaster DDLFrame field by field.
 */
void broadcaster_idle_state(FSM* fsm, Event* event);

#endif