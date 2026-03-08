#ifndef APP_SCHEDULER_H
#define APP_SCHEDULER_H

/* User library includes */
#include "util/fsm/fsm.h"
#include "status.h"

typedef eStatus (*PostFP)(uint32_t module, Event* event);

eStatus app_scheduler_init(void);

eStatus app_scheduler_subscribe(uint32_t slot, PostFP post, uint32_t module, Event* event);

eStatus app_scheduler_post(Event* event);

eStatus app_scheduler_end(void);

void app_scheduler_join(void);

void app_scheduler_delete(void);

#endif