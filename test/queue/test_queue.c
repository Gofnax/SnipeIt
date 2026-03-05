/* Standard library includes */
#include <stddef.h>

/* Third party includes */
#include "unity.h"

/* User code includes */
#include "util/queue/queue.h"

/* Tell Ceedling to inject the following sources */
TEST_SOURCE_FILE("util/queue/queue.c")

/* Mock library includes */
#include "mock_osal.h"

/* Test helpers */
static Queue my_queue;
static void* buffer[2];

static void osal_cond_wait_callback(void* cond_p, void* mutex_p, int cmock_num_calls)
{
    my_queue.size = 1;
}

void setUp(void)
{
    osal_alloc_IgnoreAndReturn(buffer);
    osal_mutex_init_IgnoreAndReturn(0);
    osal_cond_init_IgnoreAndReturn(0);
    util_queue_init(&my_queue, 2);
}

void tearDown(void) 
{
    osal_mutex_destroy_Ignore();
    osal_cond_destroy_Ignore();
    osal_dealloc_Ignore();
    util_queue_delete(&my_queue);
}

void test_queue_init(void)
{
    eStatus status = util_queue_init(NULL, 2);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    status = util_queue_init(&my_queue, 0);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    osal_alloc_IgnoreAndReturn(NULL);
    status = util_queue_init(&my_queue, 2);
    TEST_ASSERT_EQUAL(eSTATUS_SYSTEM_ERROR, status);

    osal_alloc_IgnoreAndReturn(buffer);
    osal_mutex_init_IgnoreAndReturn(1);
    osal_dealloc_Ignore();
    status = util_queue_init(&my_queue, 2);
    TEST_ASSERT_EQUAL(eSTATUS_SYSTEM_ERROR, status);

    osal_alloc_IgnoreAndReturn(buffer);
    osal_mutex_init_IgnoreAndReturn(0);
    osal_cond_init_IgnoreAndReturn(1);
    osal_dealloc_Ignore();
    status = util_queue_init(&my_queue, 2);
    TEST_ASSERT_EQUAL(eSTATUS_SYSTEM_ERROR, status);
}

void test_queue_delete(void)
{
    util_queue_delete(NULL);
}

void test_queue_push(void)
{
    osal_mutex_lock_Ignore();
    osal_cond_signal_Ignore();
    osal_mutex_unlock_Ignore();
    eStatus status = util_queue_push(&my_queue, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    status = util_queue_push(NULL, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    my_queue.buffer = NULL;
    status = util_queue_push(&my_queue, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);
    my_queue.buffer = buffer;

    osal_mutex_lock_Ignore();
    osal_mutex_unlock_Ignore();
    my_queue.size = my_queue.capacity;
    status = util_queue_push(&my_queue, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_ACTION_FAILED, status);
}

void test_queue_pop(void)
{
    void* elem;

    eStatus status = util_queue_pop(NULL, &elem);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    my_queue.buffer = NULL;
    status = util_queue_pop(&my_queue, &elem);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);
    my_queue.buffer = buffer;

    status = util_queue_pop(&my_queue, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    osal_mutex_lock_Ignore();
    osal_cond_signal_Ignore();
    osal_mutex_unlock_Ignore();
    status = util_queue_push(&my_queue, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    osal_mutex_lock_Ignore();
    osal_mutex_unlock_Ignore();
    status = util_queue_pop(&my_queue, &elem);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    osal_mutex_lock_Ignore();
    osal_mutex_unlock_Ignore();
    osal_cond_wait_Stub(osal_cond_wait_callback);
    status = util_queue_pop(&my_queue, &elem);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);
}

void test_queue_cyclic(void)
{
    int num1 = 1;
    int num2 = 2;

    osal_mutex_lock_Ignore();
    osal_cond_signal_Ignore();
    osal_mutex_unlock_Ignore();
    eStatus status = util_queue_push(&my_queue, &num1);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    osal_mutex_lock_Ignore();
    osal_cond_signal_Ignore();
    osal_mutex_unlock_Ignore();
    status = util_queue_push(&my_queue, &num2);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);
    TEST_ASSERT_TRUE(my_queue.head == my_queue.tail);

    int* outval = NULL;

    osal_mutex_lock_Ignore();
    osal_mutex_unlock_Ignore();
    status = util_queue_pop(&my_queue, &outval);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);
    TEST_ASSERT_TRUE(*outval == 1);

    osal_mutex_lock_Ignore();
    osal_mutex_unlock_Ignore();
    status = util_queue_pop(&my_queue, &outval);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);
    TEST_ASSERT_TRUE(*outval == 2);

    TEST_ASSERT_TRUE(my_queue.size == 0);
    TEST_ASSERT_TRUE(my_queue.head == my_queue.tail);
}