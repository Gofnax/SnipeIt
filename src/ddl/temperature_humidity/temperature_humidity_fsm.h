#ifndef DDL_TEMPERATURE_HUMIDITY_FSM_H
#define DDL_TEMPERATURE_HUMIDITY_FSM_H

/* User library includes */
#include "util/fsm/fsm.h"

/**
 * @brief   The initial state of the temperature and humidity sensor.
 * @details From this state we can go to temperature_humidity_error and
 *          temperature_humidity_idle states.
 * @param   fsm A pointer to an initialized FSM.
 * @param   event A pointer to an Event.
 */
void temperature_humidity_init_state(FSM* fsm, Event* event);

void temperature_humidity_error_state(FSM* fsm, Event* event);

void temperature_humidity_idle_state(FSM* fsm, Event* event);

void temperature_humidity_read_state(FSM* fsm, Event* event);

void temperature_humidity_update_state(FSM* fsm, Event* event);

#endif