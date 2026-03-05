/* Standard library includes */
#include <stddef.h>

/* Third party includes */
#include "unity.h"
#include "CException.h"

/* User code includes */
#include "util/active_object/active_object.h"

/* Tell Ceedling to inject the following sources */
TEST_SOURCE_FILE("util/active_object/active_object.c")

/* Mock library includes */
#include "mock_queue.h"
#include "mock_osal.h"
#include "mock_fsm.h"

/* Test helpers */
static ActiveObject aobj;
static EntryFP entry;
static void* arg;

static int osal_thread_create_callback(void** thread, EntryFP entry_func, void* arg_p, int cmock_num_calls)
{
    entry = entry_func;
    arg = arg_p;
    return 0;
}

static eStatus util_queue_pop_callback(Queue* queue, void** element, int cmock_num_calls)
{
    *((Event**)element) = NULL;
    return 0;
}

static void dummy_init(FSM* fsm, Event* event)
{
    (void)fsm;
    (void)event;
}

void setUp(void)
{
    util_queue_init_IgnoreAndReturn(0);
    osal_thread_create_IgnoreAndReturn(0);
    (void)util_active_object_init(&aobj, 2, dummy_init);
}

void tearDown(void) 
{
    osal_thread_join_Ignore();
    util_queue_delete_Ignore();
    util_active_object_delete(&aobj);
}

void test_active_object_entry(void)
{
    CEXCEPTION_T e;

    util_queue_init_IgnoreAndReturn(0);
    osal_thread_create_Stub(osal_thread_create_callback);
    util_fsm_init_IgnoreAndReturn(0);
    util_queue_pop_Stub(util_queue_pop_callback);
    util_fsm_send_event_ExpectAndThrow(&aobj.active_fsm, NULL, 0x55);
    (void)util_active_object_init(&aobj, 2, dummy_init);

    Try
    {
        entry(arg);
    }
    Catch(e)
    {
        TEST_ASSERT_EQUAL_HEX(0x55, e);
    }
}

void test_active_object_init(void)
{
    eStatus status = util_active_object_init(NULL, 2, dummy_init);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    status = util_active_object_init(&aobj, 2, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    util_queue_init_IgnoreAndReturn(1);
    status = util_active_object_init(&aobj, 2, dummy_init);
    TEST_ASSERT_EQUAL(eSTATUS_SYSTEM_ERROR, status);

    util_queue_init_IgnoreAndReturn(0);
    osal_thread_create_IgnoreAndReturn(1);
    util_queue_delete_Ignore();
    status = util_active_object_init(&aobj, 2, dummy_init);
    TEST_ASSERT_EQUAL(eSTATUS_SYSTEM_ERROR, status);
}

void test_active_object_post(void)
{
    Event user_event = { .type = eFSM_EVENT_USER };
    util_queue_push_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    eStatus status = util_active_object_post(&aobj, &user_event);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);
    
    status = util_active_object_post(NULL, &user_event);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    status = util_active_object_post(&aobj, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    util_queue_push_IgnoreAndReturn(eSTATUS_NULL_PARAM);
    status = util_active_object_post(&aobj, &user_event);
    TEST_ASSERT_EQUAL(eSTATUS_ACTION_FAILED, status);

    util_queue_push_IgnoreAndReturn(eSTATUS_ACTION_FAILED);
    status = util_active_object_post(&aobj, &user_event);
    TEST_ASSERT_EQUAL(eSTATUS_ACTION_FAILED, status);
}

void test_active_object_delete(void)
{
    util_active_object_delete(NULL);
}