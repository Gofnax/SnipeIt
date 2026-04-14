#ifndef UTIL_EVENT_CONFIG_H
#define UTIL_EVENT_CONFIG_H

/**
 * @brief   Active Object module identifiers.
 * @details Each Active Object module in the system must have a
 *          unique identifier listed here. Used by the event bus
 *          for routing events to the correct subscriber.
 */
typedef enum eActiveObjectID
{
    eAO_DISTANCE,
    eAO_SCHEDULER,
    eAO_COUNT
} eActiveObjectID;

#endif
