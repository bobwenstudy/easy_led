#ifndef _ELED_H_
#define _ELED_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Forward declarations */
struct eled_led;
struct eled;

#define ELED_MAX_KEYNUM (32)

/**
 * \brief           Set state state callback function
 *
 * \param[in]       led: Led instance from array to set state
 * \param[in]       state: new state
 *
 */
typedef void (*eled_set_state_fn)(struct eled_led *led, uint8_t state);

/**
 * \brief           Set state state callback function
 *
 * \param[in]       led: Led instance from array to set state
 * \param[in]       state: new state
 *
 */
typedef void (*eled_timer_callback)(void);

extern void eled_start_timer(struct eled_led *led, uint16_t time);
extern void eled_stop_timer(struct eled_led *led);

/**
 * \brief           Led Params structure
 */
typedef struct eled_led_param
{
    uint16_t time_active; /*!< LED active time in milliseconds */

    uint16_t time_inactive; /*!< LED inactive time in milliseconds */

    uint16_t blink_cnt; /*!< Total blink cnt. */

    uint16_t time_repeat_delay; /*!< LED repeat delay time in milliseconds */

    uint8_t is_repeat; /*!< Need Repeat or not */
} eled_led_param_t;

#define ELED_PARAMS_INIT(_time_active, _time_inactive, _blink_cnt, _time_repeat_delay, _is_repeat)                                                             \
    {                                                                                                                                                          \
        .time_active = _time_active, .time_inactive = _time_inactive, .blink_cnt = _blink_cnt, .time_repeat_delay = _time_repeat_delay,                        \
        .is_repeat = _is_repeat                                                                                                                                \
    }

#define ELED_LED_INIT(_led_id, _timer_handle)                                                                                                                  \
    {                                                                                                                                                          \
        .led_id = _led_id, .timer_handle = _timer_handle                                                                                                       \
    }

#define ELED_ARRAY_SIZE(_arr) sizeof(_arr) / sizeof((_arr)[0])

/**
 * \brief           led work structure
 */
typedef struct eled_led
{
    void *timer_handle; /*!< User for timer manager */

    uint16_t led_id; /*!< Current process led id */

    uint8_t blink_reserve_cnt; /*!< Current led blink reserve cnt */
    uint8_t is_in_process : 4; /*!< is in process */
    uint8_t state : 4;         /*!< Current led state */

    eled_led_param_t param;
} eled_led_t;

/**
 * \brief           easy_led group structure
 */
typedef struct eled
{
    eled_set_state_fn set_state_fn; /*!< Pointer to set state function */
} eled_t;

/**
 * \brief           Check if led is in process.
 *
 * \param[in]       led: Led handle to check
 * \return          `1` if in process, `0` otherwise
 */
int eled_is_led_in_process(const eled_led_t *led);

/**
 * \brief           Initialize led manager
 * \param[in]       set_state_fn: Pointer to function set led state on demand.
 *
 * \return          `1` on success, `0` otherwise
 */
int eled_init(eled_set_state_fn set_state_fn);

/**
 * \brief           Process the led next state.
 *
 * \param[in]       led: Led handle
 */
void eled_process_next_state(eled_led_t *led);

/**
 * \brief           Stop Led work
 *
 * \param[in]       led: Led handle
 */
void eled_stop(eled_led_t *led);

/**
 * \brief           Start Led work
 *
 * \param[in]       led: Led handle
 * \param[in]       param: Led work param
 */
void eled_start(eled_led_t *led, const eled_led_param_t *param);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ELED_H_ */
