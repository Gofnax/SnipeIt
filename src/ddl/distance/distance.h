#ifndef DDL_DISTANCE_H
#define DDL_DISTANCE_H

/* User library includes */
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

eStatus ddl_distance_init(DDLFrame* frame);

eStatus ddl_distance_post(Event* event);

eStatus ddl_distance_end(void);

void ddl_distance_join(void);

void ddl_distance_delete(void);

#endif