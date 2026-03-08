#ifndef APP_SCHEDULER_FSM_H
#define APP_SCHEDULER_FSM_H

/* User library includes */
#include "util/fsm/fsm.h"

void scheduler_init_state(FSM* fsm, Event* event);

void scheduler_error_state(FSM* fsm, Event* event);

void scheduler_idle_state(FSM* fsm, Event* event);

void scheduler_run_state(FSM* fsm, Event* event);

#endif