#ifndef APP_H
#define APP_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "app/app_config.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Initialize the APP layer.
 * @details This function calls ddl_init() from within.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      a module is misconfigured
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus app_init(void);

/**
 * @brief   Post an event to specified APP module.
 * @param   module A value from @ref eAPPModules.
 * @param   event A pointer to the event we want to send.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_INVALID_VALUE   module is out of bounds
 * @retval  eSTATUS_NULL_PARAM      event is NULL or module is uninitialized
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus app_post(uint32_t module, Event* event);

/**
 * @brief   Go to the END states of the APP modules.
 * @details This function calls ddl_end() from within.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      a module is misconfigured
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus app_end(void);

/**
 * @brief   Wait for all the APP modules to stop.
 * @details This function calls ddl_join() from within.
 */
void app_join(void);

/**
 * @brief   Free the resources of the DDL modules.
 * @details This function calls ddl_delete() from within.
 */
void app_delete(void);

#endif