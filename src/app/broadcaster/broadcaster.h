#ifndef APP_BROADCASTER_H
#define APP_BROADCASTER_H

/* User library includes */
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Configure the broadcaster module's source and destination frames.
 * @details Must be called once before app_broadcaster_init(). The pointers
 *          must remain valid for the lifetime of the module.
 * @param   source The live DDLFrame to read from.
 * @param   destination The broadcaster DDLFrame to write to.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL  successful execution
 * @retval  eSTATUS_NULL_PARAM  source or destination is NULL
 */
eStatus app_broadcaster_configure(DDLFrame* source, DDLFrame* destination);

/**
 * @brief   Initialize the broadcaster module.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_INVALID_CONFIG  app_broadcaster_configure() was not called
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initialization failed
 */
eStatus app_broadcaster_init(void);

/**
 * @brief   Post an event to the broadcaster module.
 */
eStatus app_broadcaster_post(Event* event);

/**
 * @brief   Move the broadcaster module to its END state.
 */
eStatus app_broadcaster_end(void);

/**
 * @brief   Wait for the broadcaster AO thread to stop.
 */
void app_broadcaster_join(void);

/**
 * @brief   Free broadcaster module resources.
 */
void app_broadcaster_delete(void);

#endif