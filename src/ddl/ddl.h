#ifndef DDL_H
#define DDL_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "ddl/ddl_config.h"
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Initialize the DDL modules.
 * @details Go over all the modules included in the DDL (as configured
 *          in ddl_config.h) and call the initialization functions of
 *          each.
 * @param   frame A pointer to a DDLFrame.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      frame is NULL or a module is misconfigured
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus ddl_init(DDLFrame* frame);

/**
 * @brief   Post an event to specified DDL module.
 * @param   module A value from @ref eDDLModules.
 * @param   event A pointer to the event we want to send.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_INVALID_VALUE   module is out of bounds
 * @retval  eSTATUS_NULL_PARAM      event is NULL or module is uninitialized
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus ddl_post(uint32_t module, Event* event);

/**
 * @brief   Go to the END states of the DDL modules.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      a module is misconfigured
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus ddl_end(void);

/**
 * @brief   Wait for all the DDL modules to stop.
 */
void ddl_join(void);

/**
 * @brief   Free the resources of the DDL modules.
 */
void ddl_delete(void);

#endif