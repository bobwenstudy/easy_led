#include <string.h>
#include "eled.h"

/* Default led group instance */
static eled_t eled_default;

int eled_init(eled_set_state_fn set_state_fn, eled_event_end_fn event_end_fn)
{
    eled_t *ebtobj = &eled_default;

    if (set_state_fn == NULL)
    {
        return 0;
    }

    memset(ebtobj, 0x00, sizeof(*ebtobj));
    ebtobj->set_state_fn = set_state_fn;
    ebtobj->event_end_fn = event_end_fn;

    return 1;
}

int eled_is_led_in_process(const eled_led_t *led)
{
    return led != NULL && (led->is_in_process);
}

void eled_process_end_work(eled_led_t *led)
{
    eled_t *ebtobj = &eled_default;

    led->is_in_process = 0;
    eled_stop_timer(led);

    if(ebtobj->event_end_fn)
    {
        ebtobj->event_end_fn(led);
    }
}

void eled_process_next_state(eled_led_t *led)
{
    uint8_t new_state = !led->state;

    if (!led->is_in_process)
    {
        return;
    }

#ifdef ELED_PROCESS_LAST_OFF_EVENT
    if (led->blink_reserve_cnt == 0)
    {
        eled_process_end_work(led);
        return;
    }
#endif

    // printf("eled_process_next_state(), rep: %d, process: %d, new_state: %d, rsv_cnt: %d\n", led->param.is_repeat, led->is_in_process, new_state,
    // led->blink_reserve_cnt);
    led->state = new_state;
    if (new_state)
    {
        // led on
        eled_default.set_state_fn(led, new_state);

        eled_start_timer(led, led->param.time_active);
    }
    else
    {
        // led off
        eled_default.set_state_fn(led, new_state);

        led->blink_reserve_cnt--;

        if (led->blink_reserve_cnt)
        {
            // led off
            eled_start_timer(led, led->param.time_inactive);
        }
        else
        {
            // state next cycle
            if (led->param.is_repeat)
            {
                led->blink_reserve_cnt = led->param.blink_cnt;
                // led off
                eled_start_timer(led, led->param.time_inactive + led->param.time_repeat_delay);
            }
            else
            {
#ifdef ELED_PROCESS_LAST_OFF_EVENT
                eled_start_timer(led, led->param.time_inactive);
#else
                eled_process_end_work(led);
#endif
            }
        }
    }
}

void eled_stop(eled_led_t *led)
{
    eled_stop_timer(led);

    led->is_in_process = 0;
    eled_default.set_state_fn(led, 0);
}

int eled_start(eled_led_t *led, const eled_led_param_t *param)
{
    if (led == NULL)
    {
        return 0;
    }

    // if active time is zeor,
    if ((param->time_active == 0) || (param->blink_cnt == 0))
    {
        return 0;
    }
    
    // if inactive time is zeor and blink count is more than 1
    if ((param->time_inactive == 0) && (param->blink_cnt > 1))
    {
        return 0;
    }

    // if led is in process, stop it
    if (led->is_in_process)
    {
        eled_stop(led);
    }
    
    // prepare led param, and add to led list
    memcpy(&led->param, param, sizeof(eled_led_param_t));

    led->state = 0;
    led->is_in_process = 1;
    led->blink_reserve_cnt = led->param.blink_cnt;

    eled_process_next_state(led);

    return 1;
}
