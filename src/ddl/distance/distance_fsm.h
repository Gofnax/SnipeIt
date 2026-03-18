#ifndef DDL_DISTANCE_FSM_H
#define DDL_DISTANCE_FSM_H 

/* User library includes */
#include "util/fsm/fsm.h"

/**
 * @brief   The initial state of the distance sensor.
 * @details From this state we can go to distance_error and
 *          distance_idle states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void distance_init_state(FSM* fsm, Event* event);

/**
 * @brief   The error state of the distance sensor.
 * @details This is a dead state - there is no option to go to
 *          another state from this state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void distance_error_state(FSM* fsm, Event* event);

/**
 * @brief   The idle state of the distance sensor.
 * @details This state waits to receive a read request. From this
 *          state we can go to distance_read state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void distance_idle_state(FSM* fsm, Event* event);

/**
 * @brief   The read state of the distance sensor.
 * @details This state tries to read the data from the sensor.
 *          It uses a timer to measure read timeouts. From 
 *          this state we can go to distance_update state.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void distance_read_state(FSM* fsm, Event* event);

/**
 * @brief   The update state of the distance sensor.
 * @details This state checks the validity of the data received
 *          from the distance sensor and updates the data frame 
 *          with the processed distance. From this state we can
 *          go to distance_idle and distance_read states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void distance_update_state(FSM* fsm, Event* event);

#endif