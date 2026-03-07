/* Standard library includes */
#include <stddef.h>
#include <string.h>

/* Third party includes */
#include "unity.h"

/* User code includes */
#include "ddl/distance/distance_fsm.h"
#include "ddl/distance/distance_types.h"
#include "ddl/ddl_config.h"

/* Tell Ceedling to inject the following sources */
TEST_SOURCE_FILE("ddl/distance/distance_fsm.c")

/* Mock library includes */
#include "mock_fsm.h"
#include "mock_hal_uart.h"
#include "mock_osal.h"
#include "mock_active_object.h"
#include "mock_log.h"

/* Test helpers */
DistanceObject dist_obj;
FSM            dist_fsm;
DistanceFrame  dist_frame;

TimerHandlerFP timer_callback;
void*          timer_arg;

uint8_t* read_buf;
async_cb read_callback;
void*    read_arg;

static eStatus osal_timer_init_callback(void** timer, TimerArg* arg_p, int cmock_num_calls)
{
    (void)timer;
    timer_callback = arg_p->handler;
    timer_arg = arg_p->arg;
    (void)cmock_num_calls;
    if(cmock_num_calls == 0)
    {
        return eSTATUS_SUCCESSFUL;
    }

    return eSTATUS_SYSTEM_ERROR;
}

static eStatus hal_uart_read_callback(uint32_t device, void* buf_p, size_t len, async_cb callback_fp, void* arg_p, int cmock_num_calls)
{
    (void)device;
    read_buf = buf_p;
    (void)len;
    read_callback = callback_fp;
    read_arg = arg_p;
    (void)cmock_num_calls;
    return eSTATUS_SUCCESSFUL;
}

void setUp(void)
{
    dist_obj.frame = &dist_frame;
    dist_fsm.arg = &dist_obj;
}

void tearDown(void) 
{

}

void test_distance_init_state(void)
{
    Event ev_init = { .type = eFSM_EVENT_INIT };
    log_private_Ignore();
    osal_timer_init_Stub(osal_timer_init_callback);
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_init_state(&dist_fsm, &ev_init);
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_init_state(&dist_fsm, &ev_init);
    util_active_object_post_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    timer_callback(timer_arg);

    Event ev_exit = { .type = eFSM_EVENT_EXIT };
    log_private_Ignore();
    distance_init_state(&dist_fsm, &ev_exit);

    Event ev_user = { .type = eFSM_EVENT_USER };
    log_private_Ignore();
    distance_init_state(&dist_fsm, &ev_user);
}

void test_distance_error_state(void)
{
    Event ev_entry = { .type = eFSM_EVENT_ENTRY };
    log_private_Ignore();
    distance_error_state(&dist_fsm, &ev_entry);
    TEST_ASSERT_FALSE(dist_obj.frame->valid);

    Event ev_user = { .type = eFSM_EVENT_USER };
    log_private_Ignore();
    distance_error_state(&dist_fsm, &ev_user);
}

void test_distance_idle_state(void)
{
    Event ev_entry = { .type = eFSM_EVENT_ENTRY };
    log_private_Ignore();
    dist_obj.retry = 3;
    distance_idle_state(&dist_fsm, &ev_entry);
    TEST_ASSERT_EQUAL(0, dist_obj.retry);

    Event ev_read = { .type = eDISTANCE_EVENT_READ };
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_idle_state(&dist_fsm, &ev_read);

    Event ev_exit = { .type = eFSM_EVENT_EXIT };
    log_private_Ignore();
    distance_idle_state(&dist_fsm, &ev_exit);

    Event ev_user = { .type = 100 };
    log_private_Ignore();
    distance_idle_state(&dist_fsm, &ev_user);
}

void test_distance_read_state(void)
{
    Event ev_entry = { .type = eFSM_EVENT_ENTRY };
    log_private_Ignore();
    hal_uart_write_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    hal_uart_read_Stub(hal_uart_read_callback);
    osal_timer_arm_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_read_state(&dist_fsm, &ev_entry);
    util_active_object_post_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    read_callback(read_arg);

    Event ev_frame_received = { .type = eDISTANCE_EVENT_FRAME_RECEIVED };
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_read_state(&dist_fsm, &ev_frame_received);

    Event ev_timeout = { .type = eDISTANCE_EVENT_TIMEOUT };
    log_private_Ignore();
    hal_uart_abort_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_read_state(&dist_fsm, &ev_timeout);
    dist_obj.retry = eDISTANCE_READ_RETRY_MAX;
    log_private_Ignore();
    hal_uart_abort_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    dist_obj.frame->valid = true;
    distance_read_state(&dist_fsm, &ev_timeout);
    TEST_ASSERT_FALSE(dist_obj.frame->valid);

    Event ev_exit = { .type = eFSM_EVENT_EXIT };
    log_private_Ignore();
    osal_timer_disarm_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_read_state(&dist_fsm, &ev_exit);

    Event ev_user = { .type = 100 };
    log_private_Ignore();
    distance_read_state(&dist_fsm, &ev_user);
}

void test_distance_update_state(void)
{
    Event ev_entry = { .type = eFSM_EVENT_ENTRY };
    const uint8_t rest_frame[] = { 0x57, 0x00, 0xff, 0x00, 0x9e, 0x8f, 0x00, 0x00, 0xad, 0x08, 0x00, 0x00, 0x03, 0x00, 0x06, 0x41 };
    (void)memcpy(read_buf, rest_frame, sizeof(rest_frame));
    log_private_Ignore();
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_update_state(&dist_fsm, &ev_entry);
    
    dist_obj.retry = 0;
    log_private_Ignore();
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    read_buf[0] = 0x80;
    distance_update_state(&dist_fsm, &ev_entry);
    (void)memcpy(read_buf, rest_frame, sizeof(rest_frame));

    dist_obj.retry = 0;
    log_private_Ignore();
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    read_buf[1] = 0x11;
    distance_update_state(&dist_fsm, &ev_entry);
    (void)memcpy(read_buf, rest_frame, sizeof(rest_frame)); 

    dist_obj.retry = 0;
    log_private_Ignore();
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    read_buf[3] = 0x10;
    distance_update_state(&dist_fsm, &ev_entry);
    (void)memcpy(read_buf, rest_frame, sizeof(rest_frame));

    dist_obj.retry = 0;
    log_private_Ignore();
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    distance_update_state(&dist_fsm, &ev_entry);
    (void)memcpy(read_buf, rest_frame, sizeof(rest_frame));

    dist_obj.retry = 0;
    log_private_Ignore();
    log_private_Ignore();
    util_fsm_transition_IgnoreAndReturn(eSTATUS_SUCCESSFUL);
    read_buf[sizeof(rest_frame) - 1] = 0x10;
    uint32_t temp = dist_obj.system_time;
    dist_obj.system_time = 0;
    distance_update_state(&dist_fsm, &ev_entry);
    dist_obj.system_time = temp;
    (void)memcpy(read_buf, rest_frame, sizeof(rest_frame));

    float expected_dis = 2.221f;
    TEST_ASSERT_EQUAL(36766, dist_obj.system_time);
    TEST_ASSERT_EQUAL(0, memcmp(&dist_obj.frame->distance, &expected_dis, sizeof(float)));
    TEST_ASSERT_EQUAL(0, dist_obj.frame->status);
    TEST_ASSERT_EQUAL(3, dist_obj.frame->strength);
    TEST_ASSERT_EQUAL(6, dist_obj.frame->precision);

    Event ev_exit = { .type = eFSM_EVENT_EXIT };
    log_private_Ignore();
    distance_update_state(&dist_fsm, &ev_exit);

    Event ev_user = { .type = 100 };
    log_private_Ignore();
    distance_update_state(&dist_fsm, &ev_user);
}