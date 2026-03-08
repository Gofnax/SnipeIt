#ifndef APP_H
#define APP_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "app/app_config.h"
#include "util/fsm/fsm.h"
#include "status.h"

eStatus app_init(void);

eStatus app_post(uint32_t module, Event* event);

eStatus app_end(void);

void app_join(void);

void app_delete(void);

#endif