/*
 * This file is part of The AnyKey Project  https://github.com/The-AnyKey-Project
 *
 * Copyright (c) 2021 Matthias Beckert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * keypad.c
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/hal/keypad.h"

/*
 * Include dependencies
 */
#include <string.h>
#include "api/app/anykey.h"

/*
 * Forward declarations of static functions
 */
static void _keypad_init_hal(void);
static void _keypad_init_module(void);
static void _keypad_set_sw_event(uint8_t sw_id, keypad_event_t event);

/*
 * Static variables
 */
static THD_WORKING_AREA(_keypad_poll_stack, KEYPAD_POLL_THREAD_STACK);
static keypad_event_t _keypad_events[KEYPAD_SW_COUNT];
static keypad_sw_t _keypad_sw_list[KEYPAD_SW_COUNT] = {
    {.line = KEYPAD_BTN_LINE_SW01, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW02, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW03, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW04, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW05, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW06, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW07, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW08, .delay = 0, .state = KEYPAD_SW_STATE_INIT},
    {.line = KEYPAD_BTN_LINE_SW09, .delay = 0, .state = KEYPAD_SW_STATE_INIT}};

/*
 * Global variables
 */
event_source_t keypad_event_handle;

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_keypad_poll_thread, arg)
{
  (void)arg;
  systime_t time = 0;
  uint8_t sw_id = 0;
  uint32_t pin_state = 0;
  uint8_t events_dirty = 0;

  chRegSetThreadName("keypad_poll_th");

  while (true)
  {
    time = chVTGetSystemTimeX();

    for (sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
    {
      if (_keypad_sw_list[sw_id].delay == 0)
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

        switch (_keypad_sw_list[sw_id].state)
        {
          case KEYPAD_SW_STATE_INIT:
            if (pin_state == KEYPAD_BTN_PRESSED)
            {
              _keypad_sw_list[sw_id].delay = KEYPAD_BTN_DEBOUNCE_TIME_TICKS;
              _keypad_sw_list[sw_id].state = KEYPAD_SW_STATE_PRESS;
              _keypad_set_sw_event(sw_id, KEYPAD_EVENT_PRESS);
              events_dirty = 1;
            }
            else
            {
              _keypad_set_sw_event(sw_id, KEYPAD_EVENT_NONE);
            }
            break;
          case KEYPAD_SW_STATE_PRESS:
            if (pin_state == KEYPAD_BTN_UNPRESSED)
            {
              _keypad_sw_list[sw_id].delay = KEYPAD_BTN_DEBOUNCE_TIME_TICKS;
              _keypad_sw_list[sw_id].state = KEYPAD_SW_STATE_INIT;
              _keypad_set_sw_event(sw_id, KEYPAD_EVENT_RELEASE);
              events_dirty = 1;
            }
            else
            {
              _keypad_set_sw_event(sw_id, KEYPAD_EVENT_NONE);
            }
            break;
          default:
            _keypad_sw_list[sw_id].delay = 0;
            _keypad_sw_list[sw_id].state = KEYPAD_SW_STATE_INIT;
            _keypad_set_sw_event(sw_id, KEYPAD_EVENT_NONE);
            break;
        }
      }
      else
      {
        _keypad_sw_list[sw_id].delay--;
        _keypad_set_sw_event(sw_id, KEYPAD_EVENT_NONE);
      }
    }
    if (events_dirty)
    {
      chEvtBroadcastI(&keypad_event_handle);
    }
    chThdSleepUntilWindowed(time, time + TIME_MS2I(KEYPAD_POLL_MAIN_THREAD_P_MS));
  }
}

/*
 * Static helper functions
 */
static void _keypad_set_sw_event(uint8_t sw_id, keypad_event_t event)
{
  chSysLock();
  _keypad_events[sw_id] = event;
  chSysUnlock();
}

static void _keypad_init_hal(void)
{
  uint8_t sw_id = 0;
  for (sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
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
  uint8_t sw_id = 0;
  for (sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
  {
    _keypad_set_sw_event(sw_id, KEYPAD_EVENT_NONE);
  }

  chEvtObjectInit(&keypad_event_handle);

  chThdCreateStatic(_keypad_poll_stack, sizeof(_keypad_poll_stack), KEYPAD_POLL_THREAD_PRIO,
                    _keypad_poll_thread, NULL);
}

/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void keypad_loop_switches_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    chprintf(chp, "Usage: kp-loop-sw\r\n");
    return;
  }

  event_listener_t event_listener;
  chEvtRegister(&keypad_event_handle, &event_listener, KEYPAD_EVENT_NOTIFIER_BIT);

  while (chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
    eventmask_t events;
    events = chEvtWaitOneTimeout(EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT), TIME_MS2I(100));

    if (events & EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT))
    {
      keypad_event_t dest[KEYPAD_SW_COUNT];
      keypad_get_sw_events(dest);
      uint8_t sw_id = 0;
      for (sw_id = 0; sw_id < KEYPAD_SW_COUNT; sw_id++)
      {
        if (dest[sw_id] != KEYPAD_EVENT_NONE)
        {
          char state[2];
          state[0] = (dest[sw_id] == KEYPAD_EVENT_PRESS) ? 'P' : 'R';
          state[1] = '\0';
          chprintf(chp, "SW%d%s ", (sw_id - KEYPAD_SW_ID_MIN + 1), state);
        }
      }
    }
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}
#endif

/*
 * API functions
 */
void keypad_init(void)
{
  _keypad_init_hal();
  _keypad_init_module();
}

void keypad_get_sw_events(keypad_event_t *dest)
{
  chSysLock();
  memcpy(dest, _keypad_events, sizeof(keypad_event_t) * KEYPAD_SW_COUNT);
  chSysUnlock();

  return;
}
