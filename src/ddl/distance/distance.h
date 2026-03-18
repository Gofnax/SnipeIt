#ifndef DDL_DISTANCE_H
#define DDL_DISTANCE_H

/* User library includes */
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Initialize the ToF distance measuring sensor.
 * @param   frame A pointer to the DDLFrame that will hold all our sensor data.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      frame is NULL or the DistanceObject is misconfigured
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus ddl_distance_init(DDLFrame* frame);

/**
 * @brief   Send event to the distance sensor.
 * @param   event An event from @ref eDistanceEvent.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      event is NULL or sensor is uninitialized
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus ddl_distance_post(Event* event);

/**
 * @brief   Move to the END state of the distance sensor.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      the DistanceObject is misconfigured
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus ddl_distance_end(void);

/**
 * @brief   Wait for the distance sensor to stop.
 */
void ddl_distance_join(void);

/**
 * @brief   Free the distance sensor's resources.
 */
void ddl_distance_delete(void);

#endif