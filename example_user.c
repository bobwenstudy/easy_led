#include "windows.h"
#include <stdio.h>
#include <stdlib.h>

#if 1
#include "eled.h"

#include "test_timer.h"

static LARGE_INTEGER freq, sys_start_time;
static uint32_t get_tick(void);

typedef enum
{
    USER_LED_RED = 0,
    USER_LED_GREEN,
    USER_LED_BLUE,
    USER_LED_MAX,
} user_led_t;

/* User defined settings */
static const eled_led_param_t test_param_0 = ELED_PARAMS_INIT(1, 200, 200, 5, 0, 0);
static const eled_led_param_t test_param_1 = ELED_PARAMS_INIT(2, 800, 200, 3, 0, 0);
static const eled_led_param_t test_param_2 = ELED_PARAMS_INIT(3, 3000, 2000, 1, 0, 0);

static void test_timer_timeout(void *arg)
{
    // printf("test_timer_timeout()\n");
    eled_process_next_state(arg);
}

static eled_led_t led_red;
static eled_led_t led_green;
static eled_led_t led_blue;

static struct test_timer timer_red = {NULL, 0, &led_red, test_timer_timeout};
static struct test_timer timer_green = {NULL, 0, &led_green, test_timer_timeout};
static struct test_timer timer_blue = {NULL, 0, &led_blue, test_timer_timeout};

static eled_led_t led_red = ELED_LED_INIT(USER_LED_RED, &timer_red);
static eled_led_t led_green = ELED_LED_INIT(USER_LED_GREEN, &timer_green);
static eled_led_t led_blue = ELED_LED_INIT(USER_LED_BLUE, &timer_blue);

uint32_t last_time_keys[USER_LED_MAX - USER_LED_RED] = {0};

void prv_led_set_state(struct eled_led *led, uint8_t state)
{
    uint32_t color;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    uint32_t *diff_time_ptr = &last_time_keys[led->led_id - USER_LED_RED];
    uint32_t diff_time = get_tick() - *diff_time_ptr;

    /* This is for purpose of test and timing validation */
    if (diff_time > 2000)
    {
        diff_time = 0;
    }

    *diff_time_ptr = get_tick(); /* Set current date as last one */

    if (state)
    {
        if (led == &led_red)
        {
            color = FOREGROUND_RED;
        }
        else if (led == &led_green)
        {
            color = FOREGROUND_GREEN;
        }
        else if (led == &led_blue)
        {
            color = FOREGROUND_BLUE;
        }
    }
    else
    {
        color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }

    SetConsoleTextAttribute(hConsole, color);
    printf("[%7u][%6u] ID(hex):%4x, state: %s, reserve-cnt: %3u\r\n", (unsigned)get_tick(), (unsigned)diff_time, led->led_id, state ? "ON" : "OFF",
           led->blink_reserve_cnt);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/* Process led event */
static void proc_led_end_event(struct eled_led *led)
{
    printf("[%7u] LED end event. led_id:0x%x, effect_id:0x%x\r\n", (unsigned)get_tick(), led->led_id, led->param.id);
}

void user_eled_start_timer(struct eled_led *led, uint16_t time)
{
    struct test_timer *timer = led->timer_handle;

    // printf("eled_start_timer(), time: %d\n", time);
    test_timer_start(timer, time);
}

void user_eled_stop_timer(struct eled_led *led)
{
    struct test_timer *timer = led->timer_handle;
    // printf("eled_stop_timer()");
    test_timer_stop(timer);
}

/**
 * \brief           Example function
 */
int example_user(void)
{
    uint32_t time_last;
    printf("Application running\r\n");
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&sys_start_time);

    /* Define leds */
    eled_init(prv_led_set_state, proc_led_end_event);
    eled_start(&led_red, &test_param_0);
    eled_start(&led_green, &test_param_1);
    eled_start(&led_blue, &test_param_2);

    while (1)
    {
        test_timer_polling();
        /* Artificial sleep to offload win process */
        Sleep(5);
    }
    return 0;
}

uint32_t user_test_timer_get_ticks(void)
{
    return get_tick();
}

/**
 * \brief           Get current tick in ms from start of program
 * \return          uint32_t: Tick in ms
 */
static uint32_t get_tick(void)
{
    LONGLONG ret;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    ret = now.QuadPart - sys_start_time.QuadPart;
    return (uint32_t)((ret * 1000) / freq.QuadPart);
}
#endif