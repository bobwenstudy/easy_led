#ifndef _TEST_TIMER_
#define _TEST_TIMER_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*test_timer_callback)(void *arg);

struct test_timer
{
    struct test_timer *next;

    uint32_t timeout;
    void *arg;

    test_timer_callback callback;
};

int test_timer_init(void);
int test_timer_start(struct test_timer *handle, int time);
void test_timer_stop(struct test_timer *handle);
void test_timer_polling(void);

extern uint32_t test_timer_get_ticks(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TEST_TIMER_ */
