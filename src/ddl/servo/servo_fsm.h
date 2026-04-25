#ifndef DDL_SERVO_FSM_H
#define DDL_SERVO_FSM_H

/* User library includes */
#include "util/fsm/fsm.h"

/**
 * @brief   The initial state of the servo motors.
 * @details From this state we can go to servo_error and
 *          servo_idle states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void servo_init_state(FSM* fsm, Event* event);

/**
 * @brief   The error state of the servo motors.
 * @details This is a dead state - there is no option to go to
 *          another state from this state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void servo_error_state(FSM* fsm, Event* event);

/**
 * @brief   The idle state of the servo motors.
 * @details From this state we can go to servo_error and
 *          servo_scan states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void servo_idle_state(FSM* fsm, Event* event);

/**
 * @brief   The scan state of the servo motors.
 * @details From this state we can go to servo_idle,
 *          servo_noise_sacn and servo_target_lock states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void servo_scan_state(FSM* fsm, Event* event);

/**
 * @brief   The noise scan state of the servo motors.
 * @details From this state we can go to servo_scan state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void servo_noise_scan_state(FSM* fsm, Event* event);

/**
 * @brief   The target lock state of the servo motors.
 * @details From this state we can go to servo_scan state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void servo_target_lock_state(FSM* fsm, Event* event);

#endif