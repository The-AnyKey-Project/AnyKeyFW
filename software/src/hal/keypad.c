/*
 * keypad.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

/*
 * Include ChibiOS & HAL
 */
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/*
 * Includes module API, types & config
 */
#include "api/hal/keypad.h"

/*
 * Include dependencies
 */
#include "api/app/anykey.h"

/*
 * Forward declarations of static functions
 */
static void             _keypad_init_hal        (void);
static void             _keypad_init_module     (void);
static void             _keypad_set_sw_states   (uint32_t new_state);

/*
 * Static variables
 */
static THD_WORKING_AREA(_keypad_poll_stack, KEYPAD_POLL_THREAD_STACK);
static uint32_t         _keypad_sw_states = 0;
static keypad_sw_t      _keypad_sw_list[KEYPAD_SW_COUNT] = {
  { .line = KEYPAD_BTN_LINE_SW01, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW02, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW03, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW04, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW05, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW06, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW07, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW08, .delay = 0, .state = KEYPAD_SW_STATE_INIT },
  { .line = KEYPAD_BTN_LINE_SW09, .delay = 0, .state = KEYPAD_SW_STATE_INIT }
};

/*
 * Global variables
 */
event_source_t          keypad_event_handle;

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_keypad_poll_thread, arg)
{
  (void)arg;

  chRegSetThreadName("keypad_poll_th");

  while(true)
  {
      systime_t time = chVTGetSystemTimeX();
      uint8_t sw_id = 0;
      uint32_t new_state = 0;
      uint32_t pin_state = 0;
      for(sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
      {
          if(_keypad_sw_list[sw_id].delay == 0)
          {
#if defined(USE_STLINK)
              if (_keypad_sw_list[sw_id].line == PAL_LINE(GPIOA, 14U))
              {
                pin_state = KEYPAD_BTN_UNPRESSED;
              }
              else
#endif
              {
                pin_state = palReadLine(_keypad_sw_list[sw_id].line);
              }
              if(pin_state == KEYPAD_BTN_PRESSED)
              {
                  switch(_keypad_sw_list[sw_id].state)
                  {
                    case KEYPAD_SW_STATE_INIT:
                      _keypad_sw_list[sw_id].delay = KEYPAD_BTN_PRESS_DELAY_TIME_TICKS;
                      _keypad_sw_list[sw_id].state = KEYPAD_SW_STATE_PRESS;
                      break;
                    case KEYPAD_SW_STATE_PRESS:
                      _keypad_sw_list[sw_id].delay = KEYPAD_BTN_REPRESS_DELAY_TIME_TICKS;
                      break;
                    default:
                      _keypad_sw_list[sw_id].delay = 0;
                      _keypad_sw_list[sw_id].state = KEYPAD_SW_STATE_INIT;
                      break;
                  }
                  new_state |= (1<<(sw_id + KEYPAD_SW_ID_MIN));
              }
              else
              {
                  _keypad_sw_list[sw_id].delay = 0;
                  _keypad_sw_list[sw_id].state = KEYPAD_SW_STATE_INIT;
              }
          }
          else
          {
              _keypad_sw_list[sw_id].delay--;
          }

      }
      if(new_state)
      {
          _keypad_set_sw_states(new_state);
          chEvtBroadcastI(&keypad_event_handle);
      }
      chThdSleepUntilWindowed(time, time + TIME_MS2I(KEYPAD_POLL_MAIN_THREAD_P_MS));
  }
}

/*
 * Static helper functions
 */
static void _keypad_set_sw_states(uint32_t new_state)
{
  chSysLock();
  _keypad_sw_states = new_state;
  chSysUnlock();
}

static void _keypad_init_hal(void)
{
  uint8_t sw_id = 0;
  for(sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
  {
#if defined(USE_STLINK)
    if (_keypad_sw_list[sw_id].line != PAL_LINE(GPIOA, 14U))
#endif
    {
      palSetLineMode(_keypad_sw_list[sw_id].line, KEYPAD_BTN_MODE);
    }
  }
}

static void _keypad_init_module(void)
{
  chEvtObjectInit(&keypad_event_handle);

  chThdCreateStatic(_keypad_poll_stack, sizeof(_keypad_poll_stack),
                    KEYPAD_POLL_THREAD_PRIO, _keypad_poll_thread, NULL);
}

/*
 * Callback functions
 */

/*
 * Shell functions
 */
void keypad_loop_switches_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: kp-loop-sw\r\n");
    return;
  }

  event_listener_t  event_listener;
  chEvtRegister(&keypad_event_handle, &event_listener, KEYPAD_EVENT_NOTIFIER_BIT);

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
    eventmask_t events;
    events = chEvtWaitOneTimeout(EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT),TIME_MS2I(100));

    if (events & EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT))
    {
      uint32_t states = keypad_get_sw_states();
      uint8_t sw_id = 0;
      for(sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
      {
        if (states & (1 << sw_id))
        {
#if defined(USE_STLINK)
          if (_keypad_sw_list[sw_id].line != PAL_LINE(GPIOA, 14U))
#endif
          {
            chprintf(chp, "SW%d ", (sw_id - KEYPAD_SW_ID_MIN + 1));
          }
        }
      }
    }
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

/*
 * API functions
 */
void keypad_init(void)
{
  _keypad_init_hal();
  _keypad_init_module();
}

uint32_t keypad_get_sw_states(void)
{
  uint32_t ret = 0;

  chSysLock();
  ret = _keypad_sw_states;
  chSysUnlock();

  return ret;
}
