#include <stdio.h>
#include <stdlib.h>

#include "windows.h"
#if 1
#include "eled.h"
#include "test_timer.h"

//
// Tests
//
const char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0, suites_empty = 0;
int tests_in_suite = 0, tests_run = 0, tests_failed = 0;

#define QUOTE(str) #str
#define ASSERT(x)                                                                                                                                              \
    {                                                                                                                                                          \
        tests_run++;                                                                                                                                           \
        tests_in_suite++;                                                                                                                                      \
        if (!(x))                                                                                                                                              \
        {                                                                                                                                                      \
            printf("failed assert [%s:%i] %s\n", __FILE__, __LINE__, QUOTE(x));                                                                                \
            suite_pass = 0;                                                                                                                                    \
            tests_failed++;                                                                                                                                    \
        }                                                                                                                                                      \
    }

void SUITE_START(const char *name)
{
    suite_pass = 1;
    suite_name = name;
    suites_run++;
    tests_in_suite = 0;
}

void SUITE_END(void)
{
    printf("Testing %s ", suite_name);
    size_t suite_i;
    for (suite_i = strlen(suite_name); suite_i < 80 - 8 - 5; suite_i++)
        printf(".");
    printf("%s\n", suite_pass ? " pass" : " fail");
    if (!suite_pass)
        suites_failed++;
    if (!tests_in_suite)
        suites_empty++;
}

uint32_t test_timer_get_ticks(void);

typedef enum
{
    USER_LED_RED = 0,
    USER_LED_GREEN,
    USER_LED_BLUE,
    USER_LED_MAX,
} user_led_t;

/**
 * \brief           State sequence
 */
typedef struct
{
    uint8_t state;             /*!< Set State */
    uint8_t blink_reserve_cnt; /*!< Number of led reserve blink cnt */
    uint16_t duration;         /*!< Time */
} led_test_evt_t;

typedef struct
{
    const char *test_name;
    uint16_t test_led_id;
    const eled_led_param_t *test_param;
    int test_events_cnt;
    const led_test_evt_t *test_events; /*!< Time until this state is enabled */
} led_test_arr_t;

#define TEST_ARRAY_DEFINE(_led_id, _evt, _param)                                                                                                               \
    {                                                                                                                                                          \
        .test_led_id = _led_id, .test_name = #_evt, .test_param = _param, .test_events_cnt = ELED_ARRAY_SIZE(_evt), .test_events = _evt                        \
    }

/* Max number of ms to demonstrate */
#define MAX_TIME_MS 0x1FFF

static void test_timer_timeout(void *arg)
{
    //     printf("test_timer_timeout()\n");
    eled_process_next_state(arg);
}

static eled_led_t leds[3];

struct test_timer timer_red = {NULL, 0, &leds[0], test_timer_timeout};
struct test_timer timer_green = {NULL, 0, &leds[1], test_timer_timeout};
struct test_timer timer_blue = {NULL, 0, &leds[2], test_timer_timeout};

/* List of used leds -> test case */
static eled_led_t leds[] = {
        ELED_LED_INIT(USER_LED_RED, &timer_red),
        ELED_LED_INIT(USER_LED_GREEN, &timer_green),
        ELED_LED_INIT(USER_LED_BLUE, &timer_blue),
};

static volatile uint32_t test_processed_time_current;

/* Set led state -> used for test purposes */
#define LED_STATE_RAW(_state_, _blink_reserve_cnt_, _duration_)                                                                                                \
    {                                                                                                                                                          \
        .state = (_state_), .blink_reserve_cnt = (_blink_reserve_cnt_), .duration = (_duration_)                                                               \
    }

/*
 * Simulate led event
 */
static const eled_led_param_t test_param_single = ELED_PARAMS_INIT(100, 200, 0, 0, 0);
static const led_test_evt_t test_events_single[] = {
        LED_STATE_RAW(1, 0, 0),
        LED_STATE_RAW(0, 0, test_param_single.time_active),
};

static const eled_led_param_t test_param_single_on_zero = ELED_PARAMS_INIT(0, 200, 0, 0, 0);
static const led_test_evt_t test_events_single_on_zero[] = {};

static const eled_led_param_t test_param_single_off_zero = ELED_PARAMS_INIT(100, 0, 0, 0, 0);
static const led_test_evt_t test_events_single_off_zero[] = {};

static const eled_led_param_t test_param_single_repeat = ELED_PARAMS_INIT(100, 200, 0, 1000, 1);
static const led_test_evt_t test_events_single_repeat[] = {
        LED_STATE_RAW(1, 0, 0),
        LED_STATE_RAW(0, 0, test_param_single_repeat.time_active),
};

static const eled_led_param_t test_param_blink = ELED_PARAMS_INIT(100, 200, 1, 0, 0);
static const led_test_evt_t test_events_blink[] = {
        LED_STATE_RAW(1, 1, 0),
        LED_STATE_RAW(0, 1, test_param_blink.time_active),
        LED_STATE_RAW(1, 0, test_param_blink.time_inactive),
        LED_STATE_RAW(0, 0, test_param_blink.time_active),
};

static const eled_led_param_t test_param_blink_on_zero = ELED_PARAMS_INIT(0, 200, 1, 0, 0);
static const led_test_evt_t test_events_blink_on_zero[] = {};

static const eled_led_param_t test_param_blink_off_zero = ELED_PARAMS_INIT(100, 0, 1, 0, 0);
static const led_test_evt_t test_events_blink_off_zero[] = {};

static const eled_led_param_t test_param_blink_repeat = ELED_PARAMS_INIT(200, 300, 1, 1000, 1);
static const led_test_evt_t test_events_blink_repeat[] = {
        LED_STATE_RAW(1, 1, 0),
        LED_STATE_RAW(0, 1, test_param_blink_repeat.time_active),
        LED_STATE_RAW(1, 0, test_param_blink_repeat.time_inactive),
        LED_STATE_RAW(0, 0, test_param_blink_repeat.time_active),
};

static led_test_arr_t test_list[] = {
        TEST_ARRAY_DEFINE(USER_LED_RED, test_events_single, &test_param_single),
        TEST_ARRAY_DEFINE(USER_LED_RED, test_events_single_on_zero, &test_param_single_on_zero),
        TEST_ARRAY_DEFINE(USER_LED_RED, test_events_single_off_zero, &test_param_single_off_zero),
        TEST_ARRAY_DEFINE(USER_LED_RED, test_events_single_repeat, &test_param_single_repeat),

        TEST_ARRAY_DEFINE(USER_LED_GREEN, test_events_blink, &test_param_blink),
        TEST_ARRAY_DEFINE(USER_LED_GREEN, test_events_blink_on_zero, &test_param_blink_on_zero),
        TEST_ARRAY_DEFINE(USER_LED_GREEN, test_events_blink_off_zero, &test_param_blink_off_zero),
        TEST_ARRAY_DEFINE(USER_LED_GREEN, test_events_blink_repeat, &test_param_blink_repeat),
};

static led_test_arr_t *select_test_item;

static uint32_t test_get_state_total_duration(void)
{
    uint32_t duration = 0;

    for (size_t i = 0; i < select_test_item->test_events_cnt; ++i)
    {
        if (select_test_item->test_events[i].duration == 0)
        {
            continue;
        }
        duration += select_test_item->test_events[i].duration + 1; /* Advance time, need add 1 for state switch time. */
    }

    return duration;
}

struct eled_led *test_get_led_by_led_id(uint16_t id)
{
    for (int i = 0; i < ELED_ARRAY_SIZE(leds); i++)
    {
        if (leds[i].led_id == id)
        {
            return &leds[i];
        }
    }

    return NULL;
}

static uint32_t test_processed_event_time_prev;
static uint32_t test_processed_array_index = 0;
/* Process led event */
static void prv_led_set_state(struct eled_led *led, uint8_t state)
{
    const char *s;
    uint32_t color, keepalive_cnt = 0, diff_time;
    const led_test_evt_t *test_evt_data = NULL;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    uint16_t duration = 0;

    // printf("test_processed_array_index: %d\n", test_processed_array_index);
    if (test_processed_array_index >= select_test_item->test_events_cnt)
    {
        if (led->param.is_repeat)
        {
            test_processed_array_index = 0;

            test_evt_data = &select_test_item->test_events[test_processed_array_index];

            duration = led->param.time_inactive + led->param.time_repeat_delay;
        }
        else
        {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("[%7u] ERROR! Array index is out of bounds!\r\n", (unsigned)test_processed_time_current);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }
    else
    {
        test_evt_data = &select_test_item->test_events[test_processed_array_index];

        duration = test_evt_data->duration;
    }

    /* Handle timing */
    diff_time = test_processed_time_current - test_processed_event_time_prev;
    test_processed_event_time_prev = test_processed_time_current;

    // printf("duration: %d, diff: %d, time_active: %d, time_inactive: %d, time_repeat_delay: %d\n", duration, diff_time, led->param.time_active,
    // led->param.time_inactive, led->param.time_repeat_delay);

    /* Event type must match */
    ASSERT((test_evt_data != NULL) && (test_evt_data->state == state));
    ASSERT((test_evt_data != NULL) && (test_evt_data->blink_reserve_cnt == led->blink_reserve_cnt));
    ASSERT((test_evt_data != NULL) && (duration == diff_time));

    /* Get event string */
    if (state)
    {
        if (led->led_id == USER_LED_RED)
        {
            color = FOREGROUND_RED;
        }
        else if (led->led_id == USER_LED_GREEN)
        {
            color = FOREGROUND_GREEN;
        }
        else if (led->led_id == USER_LED_BLUE)
        {
            color = FOREGROUND_BLUE;
        }
    }
    else
    {
        color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    SetConsoleTextAttribute(hConsole, color);
    printf("[%7u][%6u] ID(hex):%4x, state: %s, reserve-cnt: %3u\r\n", (unsigned)test_timer_get_ticks(), (unsigned)diff_time, led->led_id, state ? "ON" : "OFF",
           led->blink_reserve_cnt);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    ++test_processed_array_index; /* Go to next step in next event */
}

void eled_start_timer(struct eled_led *led, uint16_t time)
{
    struct test_timer *timer = led->timer_handle;

    //     printf("eled_start_timer(), time: %d\n", time);
    test_timer_start(timer, time);
}

void eled_stop_timer(struct eled_led *led)
{
    struct test_timer *timer = led->timer_handle;
    //     printf("eled_stop_timer()");
    test_timer_stop(timer);
}

/**
 * \brief           Test function
 */
int example_test(void)
{
    printf("Test running\r\n");

    for (int index = 0; index < ELED_ARRAY_SIZE(test_list); index++)
    {
        select_test_item = &test_list[index];

        SUITE_START(select_test_item->test_name);

        // printf("\n");

        // init variable
        test_processed_event_time_prev = 0;
        test_processed_array_index = 0;
        test_processed_time_current = 0;

        test_timer_init();

        /* Define leds */
        eled_init(prv_led_set_state);

        eled_start(test_get_led_by_led_id(select_test_item->test_led_id), select_test_item->test_param);

        /* Counter simulates ms tick */
        for (uint32_t i = 0; i < MAX_TIME_MS; ++i)
        {
            test_processed_time_current = i; /* Set current time used in callback */

            test_timer_polling();

            // printf("time: %d, end: %d\n", i, test_processed_array_index >= select_test_item->test_events_cnt);
            // test_timer_print();
            // check end
            if (!test_get_led_by_led_id(select_test_item->test_led_id)->param.is_repeat)
            {
                if (test_processed_array_index >= select_test_item->test_events_cnt)
                {
                    uint32_t duration = test_get_state_total_duration();
                    if (i > duration + 1)
                    {
                        ASSERT(!test_get_led_by_led_id(select_test_item->test_led_id)->is_in_process);
                    }
                }
            }
        }
        if (!test_get_led_by_led_id(select_test_item->test_led_id)->param.is_repeat)
        {
            ASSERT(test_processed_array_index == select_test_item->test_events_cnt);
        }

        // printf("\n");

        SUITE_END();
    }
    return 0;
}

uint32_t test_timer_get_ticks(void)
{
    return test_processed_time_current;
}
#endif