#ifndef UTIL_ACTIVE_OBJECT_H
#define UTIL_ACTIVE_OBJECT_H

/* Standard libraries */
#include <stdint.h>

/* User libraries */
#include "util/queue/queue.h"
#include "util/fsm/fsm.h"
#include "status.h"

typedef struct
{
    void*   thread;
    Queue   event_queue;
    FSM     active_fsm;
    StateFP init_state;
} ActiveObject;

/**
 * @brief   Initialize an Active Object.
 * @details Initializes a queue and creates a thread for the Active Object.
 * @param   active_object A pointer to an uninitialized ActiveObject struct.
 * @param   capacity The number of slots to be in the AO's queue.
 * @param   init_state A function pointer to the initial FSM state.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      active_object or init_state are NULL
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus util_active_object_init(ActiveObject* active_object, uint32_t capacity, StateFP init_state);

/**
 * @brief   Send an event to an Active Object.
 * @param   active_object A pointer to an initialized ActiveObject struct.
 * @param   event An event from @ref eFSMEvent (to be expanded for practical use).
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      active_object or event are NULL
 * @retval  eSTATUS_ACTION_FAILED   thread or queue initalization failed
 */
eStatus util_active_object_post(ActiveObject* active_object, Event* event);

/**
 * @brief   Free an Active Object.
 * @param   active_object A pointer to an initialized ActiveObject struct.
 */
void util_active_object_delete(ActiveObject* active_object);

#endif