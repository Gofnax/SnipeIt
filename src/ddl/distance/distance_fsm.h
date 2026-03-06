#ifndef DDL_DISTANCE_FSM_H
#define DDL_DISTANCE_FSM_H 

/* User library includes */
#include "util/fsm/fsm.h"

void distance_init_state(FSM* fsm, Event* event);

void distance_error_state(FSM* fsm, Event* event);

void distance_idle_state(FSM* fsm, Event* event);

void distance_read_state(FSM* fsm, Event* event);

void distance_update_state(FSM* fsm, Event* event);

#endif