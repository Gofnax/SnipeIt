#ifndef APP_SCHEDULER_FSM_H
#define APP_SCHEDULER_FSM_H

/* User library includes */
#include "util/fsm/fsm.h"

/**
 * @brief   The initial state of the scheduler.
 * @details From this state we can go to scheduler_error and
 *          scheduler_idle states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void scheduler_init_state(FSM* fsm, Event* event);

/**
 * @brief   The error state of the scheduler.
 * @details This is a dead state - there is no option to go to
 *          another state from this state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void scheduler_error_state(FSM* fsm, Event* event);

/**
 * @brief   The idle state of the scheduler.
 * @details This state waits to receive a start event. From this
 *          state we can go to scheduler_run state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void scheduler_idle_state(FSM* fsm, Event* event);

/**
 * @brief   The running state of the scheduler.
 * @details This state arms a timer on entry and triggers the
 *          modules subscribed to it according to the event they
 *          subscribed with. From this state we can go to
 *          scheduler_idle state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void scheduler_run_state(FSM* fsm, Event* event);

#endif