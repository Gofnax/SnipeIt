/* Third party includes */
#include "unity.h"

/* User code includes */
#include "util/fsm/fsm.h"

/* Tell Ceedling to inject the following sources */
TEST_SOURCE_FILE("util/fsm/fsm.c")

/* Test helpers */
static void dummy_state(FSM* fsm, Event* event)
{
    (void)fsm;
    (void)event;
}

static FSM my_fsm;

void setUp(void)
{
    my_fsm.current_state = dummy_state;
    my_fsm.arg           = NULL;
}

void tearDown(void) 
{
    /* No teardown needed */
}

void test_fsm_init(void)
{
    eStatus status = util_fsm_init(&my_fsm, dummy_state, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    status = util_fsm_init(NULL, dummy_state, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    status = util_fsm_init(&my_fsm, NULL, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    status = util_fsm_init(NULL, NULL, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);
}

void test_fsm_send_event(void)
{
    Event dummy_event = { .type = eFSM_EVENT_USER };

    eStatus status = util_fsm_send_event(&my_fsm, &dummy_event);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    status = util_fsm_send_event(NULL, &dummy_event);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    my_fsm.current_state = NULL;

    status = util_fsm_send_event(&my_fsm, &dummy_event);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);
}

void test_fsm_transition(void)
{
    eStatus status = util_fsm_transition(&my_fsm, dummy_state);
    TEST_ASSERT_EQUAL(eSTATUS_SUCCESSFUL, status);

    status = util_fsm_transition(NULL, dummy_state);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    status = util_fsm_transition(&my_fsm, NULL);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);

    my_fsm.current_state = NULL;
    status = util_fsm_transition(&my_fsm, dummy_state);
    TEST_ASSERT_EQUAL(eSTATUS_NULL_PARAM, status);
}