#ifndef APP_SCHEDULER_H
#define APP_SCHEDULER_H

/* User library includes */
#include "util/fsm/fsm.h"
#include "status.h"

typedef eStatus (*PostFP)(uint32_t module, Event* event);

/**
 * @brief   Initialize the scheduler.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus app_scheduler_init(void);

/**
 * @brief   Subscribe to the scheduling service.
 * @details The scheduling service holds an array of time slots it triggers
 *          preodically. Each slot needs to be subscribed to by a module to
 *          time the events it requires to operate.
 * @param   slot The time slot (index) the module subscribes to.
 * @param   post A function pointer to the post function of the DDL (as in ddl.h).
 * @param   module The module index (as determined in @ref eDDLModules).
 * @param   event The event that will be sent to the subscribing module.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_INVALID_VALUE   slot is out of bounds
 * @retval  eSTATUS_NULL_PARAM      post or event are NULL
 * @retval  eSTATUS_ACTION_FAILED   slot is already taken by another module
 */
eStatus app_scheduler_subscribe(uint32_t slot, PostFP post, uint32_t module, Event* event);

/**
 * @brief   Send event to the scheduler.
 * @param   event An event from @ref eDistanceEvent.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      event is NULL or scheduler is uninitialized
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed 
 */
eStatus app_scheduler_post(Event* event);

/**
 * @brief   Move to the END state of the scheduler.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      scheduler is uninitialized
 * @retval  eSTATUS_ACTION_FAILED   thread or queue action failed
 */
eStatus app_scheduler_end(void);

/**
 * @brief   Wait for an scheduler to stop.
 */
void app_scheduler_join(void);

/**
 * @brief   Free the scheduler resources.
 */
void app_scheduler_delete(void);

#endif