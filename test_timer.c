#include "windows.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "test_timer.h"

extern uint32_t test_timer_get_ticks(void);

struct test_timer *head_handle;

int test_timer_init(void)
{
    head_handle = NULL;
    return 0;
}

int test_timer_start(struct test_timer *handle, int timeout)
{
    struct test_timer *target = head_handle;

    handle->timeout = test_timer_get_ticks() + timeout;
    while (target)
    {
        if (target == handle)
            return -1; // already exist.
        target = target->next;
    }
    handle->next = head_handle;
    head_handle = handle;
    return 0;
}

void test_timer_stop(struct test_timer *handle)
{
    struct test_timer **curr;
    for (curr = &head_handle; *curr;)
    {
        struct test_timer *entry = *curr;
        if (entry == handle)
        {
            *curr = entry->next;
            return;
        }
        else
        {
            curr = &entry->next;
        }
    }
}

void test_timer_polling(void)
{
    struct test_timer *target;
    for (target = head_handle; target; target = target->next)
    {
        if (target->timeout <= test_timer_get_ticks())
        {
            target->callback(target->arg);
        }
    }
}

void test_timer_print(void)
{
    struct test_timer *target;
    for (target = head_handle; target; target = target->next)
    {
        printf("cur: %d, timeout: %d\n", test_timer_get_ticks(), target->timeout);
    }
}
