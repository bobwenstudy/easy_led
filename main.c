#include <stdio.h>
#include <string.h>
#include "eled.h"
#include "windows.h"

#define FUNC_TESTER_WORK

extern int example_test(void);
extern int example_user(void);

extern void tester_eled_start_timer(struct eled_led *led, uint16_t time);
extern void user_eled_start_timer(struct eled_led *led, uint16_t time);

extern void tester_eled_stop_timer(struct eled_led *led);
extern void user_eled_stop_timer(struct eled_led *led);

extern int tester_test_timer_get_ticks(void);
extern int user_test_timer_get_ticks(void);

void eled_start_timer(struct eled_led *led, uint16_t time)
{
#ifdef FUNC_TESTER_WORK
    tester_eled_start_timer(led, time);
#else
    user_eled_start_timer(led, time);
#endif
}

void eled_stop_timer(struct eled_led *led)
{
#ifdef FUNC_TESTER_WORK
    tester_eled_stop_timer(led);
#else
    user_eled_stop_timer(led);
#endif
}

uint32_t test_timer_get_ticks(void)
{
#ifdef FUNC_TESTER_WORK
    return tester_test_timer_get_ticks();
#else
    return user_test_timer_get_ticks();
#endif
}

int main(void)
{
#ifdef FUNC_TESTER_WORK
    example_test();
#else
    example_user();
#endif
    return 0;
}
