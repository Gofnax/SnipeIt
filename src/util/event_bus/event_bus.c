#include "event_bus.h"

/* Standard library includes */
#include <stddef.h>
#include <stdbool.h>

/* User library includes */
#include "util/event_bus/event_bus_config.h"
#include "util/log/log.h"
#include "osal/osal.h"

typedef struct
{
    EventBusPostFP  post_fn;
    Event*          event;
    eActiveObjectID ao_id;
    uint32_t        module;
    bool            active;
    uint8_t         reserved[7];
} Subscription;

static Subscription subscriptions[eEVENT_BUS_MAX_SUBSCRIPTIONS];
static uint32_t     subscription_count;
static void*        bus_mutex;

eStatus util_event_bus_init(void)
{
    LOG_DEBUG("Initializing the event bus");
    subscription_count = 0;

    for(uint32_t i = 0; i < eEVENT_BUS_MAX_SUBSCRIPTIONS; i++)
    {
        subscriptions[i].active = false;
    }

    if(osal_mutex_init(&bus_mutex))
    {
        return eSTATUS_SYSTEM_ERROR;
    }

    return eSTATUS_SUCCESSFUL;
}

eStatus util_event_bus_subscribe(eActiveObjectID ao_id, EventBusPostFP post_fn,
                                    uint32_t module, Event* event)
{
    if(ao_id >= eAO_COUNT)
    {
        return eSTATUS_INVALID_VALUE;
    }

    if(post_fn == NULL || event == NULL)
    {
        return eSTATUS_NULL_PARAM;
    }

    LOG_DEBUG("Active object ID %d subscribed to the event bus with event %u",
                ao_id, event->type);
    osal_mutex_lock(bus_mutex);

    if(subscription_count >= eEVENT_BUS_MAX_SUBSCRIPTIONS)
    {
        osal_mutex_unlock(bus_mutex);
        return eSTATUS_ACTION_FAILED;
    }

    // Find the first inactive slot
    for(uint32_t i = 0; i < eEVENT_BUS_MAX_SUBSCRIPTIONS; i++)
    {
        if(!subscriptions[i].active)
        {
            subscriptions[i].ao_id = ao_id;
            subscriptions[i].post_fn = post_fn;
            subscriptions[i].module = module;
            subscriptions[i].event = event;
            subscriptions[i].active = true;
            subscription_count++;

            osal_mutex_unlock(bus_mutex);
            return eSTATUS_SUCCESSFUL;
        }
    }

    osal_mutex_unlock(bus_mutex);
    return eSTATUS_ACTION_FAILED;
}

eStatus util_event_bus_publish(eActiveObjectID ao_id, uint32_t event_type)
{
    if(ao_id >= eAO_COUNT)
    {
        return eSTATUS_INVALID_VALUE;
    }

    bool matched = false;

    osal_mutex_lock(bus_mutex);

    for(uint32_t i = 0; i < eEVENT_BUS_MAX_SUBSCRIPTIONS; i++)
    {
        if(subscriptions[i].active && subscriptions[i].ao_id == ao_id &&
           subscriptions[i].event->type == event_type)
        {
            /* Call the subscriber's layer's post function with its stored event */
            subscriptions[i].post_fn(subscriptions[i].module, subscriptions[i].event);
            matched = true;
        }
    }

    osal_mutex_unlock(bus_mutex);

    return matched ? eSTATUS_SUCCESSFUL : eSTATUS_ACTION_FAILED;
}

void util_event_bus_destroy(void)
{
    osal_mutex_lock(bus_mutex);

    for(uint32_t i = 0; i < eEVENT_BUS_MAX_SUBSCRIPTIONS; i++)
    {
        subscriptions[i].active = false;
    }

    subscription_count = 0;

    osal_mutex_unlock(bus_mutex);
    osal_mutex_destroy(bus_mutex);
}