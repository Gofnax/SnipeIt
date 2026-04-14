#ifndef UTIL_EVENT_BUS_H
#define UTIL_EVENT_BUS_H

/* Standard library includes */
#include <stdint.h>

/* User library includes */
#include "util/event_bus/event_config.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Function pointer type for posting events to an Active Object.
 * @details Matches the signature of module post functions
 *          (e.g., ddl_distance_post).
 */
typedef eStatus (*EventBusPostFP)(uint32_t module, Event* event);

/**
 * @brief   Initialize the event bus.
 * @details Must be called once before any subscribe or publish calls.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_SYSTEM_ERROR    mutex initialization failed
 */
eStatus util_event_bus_init(void);

/**
 * @brief   Subscribe to an event on the bus.
 * @details Registers a callback to be invoked when the specified event
 *          is published for the given Active Object module.
 * @param   ao_id The Active Object module ID from @ref eActiveObjectID.
 * @param   post_fn A function pointer to the module's post function.
 * @param   module The index of the AO module in its layer (doesn't check for
 *          index out-of-bouds).
 * @param   event A pointer to a static Event that will be passed to post_fn
 *          on publish. Must remain valid for the lifetime of the subscription.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_INVALID_VALUE   ao_id is out of bounds
 * @retval  eSTATUS_NULL_PARAM      post_fn or event are NULL
 * @retval  eSTATUS_ACTION_FAILED   subscription table is full
 */
eStatus util_event_bus_subscribe(eActiveObjectID ao_id, EventBusPostFP post_fn,
                                    uint32_t module, Event* event);

/**
 * @brief   Publish an event through the bus.
 * @details Looks up all subscriptions matching the given parameter pair and
 *          invokes each subscriber's post function with its stored event pointer.
 * @param   ao_id The target Active Object module identifier from @ref eActiveObjectID.
 * @param   event_type The event type to publish (from a module's event enum).
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution, at least one subscriber notified
 * @retval  eSTATUS_INVALID_VALUE   ao_id is out of bounds
 * @retval  eSTATUS_ACTION_FAILED   no matching subscription found
 */
eStatus util_event_bus_publish(eActiveObjectID ao_id, uint32_t event_type);

/**
 * @brief   Destroy the event bus.
 * @details Deactivates all subscriptions and frees resources.
 */
void util_event_bus_destroy(void);

#endif