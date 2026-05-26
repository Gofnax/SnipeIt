#ifndef DDL_GPS_H
#define DDL_GPS_H

/* User library includes */
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

eStatus ddl_gps_init(DDLFrame* frame);

eStatus ddl_gps_post(Event* event);

eStatus ddl_gps_end(void);

void ddl_gps_join(void);

void ddl_gps_delete(void);

#endif