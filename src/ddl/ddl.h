#ifndef DDL_H
#define DDL_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "ddl/ddl_config.h"
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

eStatus ddl_init(DDLFrame* frame);

eStatus ddl_post(uint32_t module, Event* event);

eStatus ddl_end(void);

void ddl_join(void);

void ddl_delete(void);

#endif