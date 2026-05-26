#ifndef DDL_GPS_FSM_H
#define DDL_GPS_FSM_H

/* User library includes */
#include "util/fsm/fsm.h"

void gps_init_state(FSM* fsm, Event* event);

void gps_error_state(FSM* fsm, Event* event);

void gps_idle_state(FSM* fsm, Event* event);

void gps_read_state(FSM* fsm, Event* event);

void gps_update_state(FSM* fsm, Event* event);

#endif