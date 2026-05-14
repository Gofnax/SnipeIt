#ifndef DDL_TEMPERATURE_HUMIDITY__H
#define DDL_TEMPERATURE_HUMIDITY__H

/* User library includes */
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Initialize the temperature and humidity measuring sensor.
 * @param   frame A pointer to the DDLFrame that will hold all our sensor data.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      frame is NULL or the TemperatureHumidityObject is misconfigured
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus ddl_temperature_humidity_init(DDLFrame* frame);

/**
 * @brief   Send event to the temperature & humidity sensor.
 * @param   event An event from @ref eTemperatureHumidityEvent.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      event is NULL or sensor is uninitialized
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus ddl_temperature_humidity_post(Event* event);

/**
 * @brief   Move to the END state of the temperature & humidity sensor.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      the TemperatureHumidityObject is misconfigured
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus ddl_temperature_humidity_end(void);

/**
 * @brief   Wait for the temperature & humidity sensor to stop.
 */
void ddl_temperature_humidity_join(void);

/**
 * @brief   Free the temperature & humidity sensor's resources.
 */
void ddl_temperature_humidity_delete(void);

#endif