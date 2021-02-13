/*
 * anykey.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
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
#include "api/app/anykey.h"

/*
 * Include dependencies
 */
#include "api/app/cmd_shell.h"
#include "api/hal/glcd.h"
#include "api/hal/keypad.h"
#include "api/hal/usb.h"
#include <assert.h>

/*
 * Static asserts
 */
static_assert(ANYKEY_NUMBER_OF_KEYS == (KEYPAD_SW_COUNT),
              "Number of keys does not match number used keypad switches");
static_assert(ANYKEY_NUMBER_OF_KEYS == (GLCD_DISP_MAX),
              "Number of keys does not match number used displays");

/*
 * Forward declarations of static functions
 */
static void _anykey_init_hal(void);
static void _anykey_init_module(void);
static void _anykey_set_layer(anykey_layer_t *layer);
static void _anykey_handle_action(anykey_action_list_t *action_list, keypad_event_t event);

/*
 * Static variables
 */
static THD_WORKING_AREA(_anykey_key_stack, ANYKEY_KEY_THREAD_STACK);
static THD_WORKING_AREA(_anykey_cmd_stack, ANYKEY_CMD_THREAD_STACK);
static anykey_layer_t *_anykey_current_layer = (anykey_layer_t *)NULL;
static uint8_t _anykey_current_display_contrast[ANYKEY_NUMBER_OF_KEYS];

/*
 * Global variables
 */

/*
 * Tasks
 */

static __attribute__((noreturn)) THD_FUNCTION(_anykey_key_thread, arg)
{
  (void)arg;

  chRegSetThreadName("anykey_key_th");

  event_listener_t event_listener;
  chEvtRegister(&keypad_event_handle, &event_listener, KEYPAD_EVENT_NOTIFIER_BIT);

  while (true)
  {
    eventmask_t events = chEvtWaitAny(EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT));
    if (events & EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT))
    {
      keypad_event_t dest[ANYKEY_NUMBER_OF_KEYS];
      keypad_get_sw_events(dest);
      uint8_t sw_id = 0;
      for (sw_id = 0; sw_id < ANYKEY_NUMBER_OF_KEYS; sw_id++)
      {
        switch (dest[sw_id])
        {
          case KEYPAD_EVENT_PRESS:
            _anykey_handle_action(_anykey_current_layer->key_actions_press[sw_id]);
            break;
          case KEYPAD_EVENT_RELEASE:
            _anykey_handle_action(_anykey_current_layer->key_actions_release[sw_id]);
            break;
          case KEYPAD_EVENT_NONE:
          default:
            break;
        }
      }
    }
  }
}

static __attribute__((noreturn)) THD_FUNCTION(_anykey_cmd_thread, arg)
{
  (void)arg;

  chRegSetThreadName("anykey_cmd_th");

  //  while(true)
  //  {
  // TODO
  //  }
}

/*
 * Static helper functions
 */

static void _anykey_set_layer(anykey_layer_t *layer)
{
  if (layer)
  {
    chSysLock();
    _anykey_current_layer = layer;
    chSysUnlock();
    glcd_set_displays(layer->displays);
  }
}

static void _anykey_handle_action(anykey_action_list_t *action_list)
{
  uint8_t i = 0;
  while (i < action_list->length)
  {
    switch (action_list->actions[i])
    {
      case ANYKEY_ACTION_KEY_PRESS:
        // TODO
        i += 2;
        break;
      case ANYKEY_ACTION_KEYEXT_PRESS:
        // TODO
        i += 3;
        break;
      case ANYKEY_ACTION_KEY_RELEASE:
        // TODO
        i += 2;
        break;
      case ANYKEY_ACTION_KEYEXT_RELEASE:
        // TODO
        i += 3;
        break;
      case ANYKEY_ACTION_NEXT_LAYER:
        _anykey_set_layer(_anykey_current_layer->next);
        i += 1;
        break;
      case ANYKEY_ACTION_PREV_LAYER:
        _anykey_set_layer(_anykey_current_layer->prev);
        i += 1;
        break;
      case ANYKEY_ACTION_ADJUST_CONTRAST:
        int8_t adjust = *((int8_t *)&action_list->actions[i]);
        uint8_t sw_id = 0;
        for (sw_id = 0; sw_id < ANYKEY_NUMBER_OF_KEYS; sw_id++)
        {
          int16_t new_value = _anykey_current_display_contrast[sw_id];
          new_value += adjust;
          new_value = (new_value > 255) ? 255 : ((new_value < 0) ? 0 : new_value);
          glcd_set_contrast((glcd_display_id_t)sw_id, (uint8_t)new_value);
        }
        i += 2;
        break;
      default:
        i++;
        break;
    }
  }
}

static void _anykey_init_hal(void)
{
#if defined(USE_STLINK)
  AFIO->MAPR |= (2 << 24);  // Disable NJTRST, allow SWD
#else
  AFIO->MAPR |= (4 << 24);  // Disable SWD, use all pins as GPIO
#endif
}

static void _anykey_init_module(void)
{
  chThdCreateStatic(_anykey_key_stack, sizeof(_anykey_key_stack), ANYKEY_KEY_THREAD_PRIO,
                    _anykey_key_thread, NULL);
  chThdCreateStatic(_anykey_cmd_stack, sizeof(_anykey_cmd_stack), ANYKEY_CMD_THREAD_PRIO,
                    _anykey_cmd_thread, NULL);
}

/*
 * Callback functions
 */

/*
 * Shell functions
 */

void anykey_loop_keys_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void anykey_show_layer_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void anykey_list_layers_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void anykey_set_layer_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

/*
 * API functions
 */

void anykey_init(void)
{
  /*
   * Toplevel hal init
   * (disable jtag functionality)
   */
  _anykey_init_hal();

  /*
   * Project specific hal initialization
   */
  glcd_init();
  keypad_init();
  usb_init();

#if defined(USE_CMD_SHELL)
  /*
   * Initialize additional application components
   */
  cmd_shell_init();
#endif

  /*
   * Initialize toplevel application
   */
  _anykey_init_module();

#if defined(USE_CMD_SHELL)
  /*
   * Start shell handling,
   * should never return
   */
  cmd_shell_loop();
#else
  /*
   * Nothing to do, kill initial thread
   */
  chThdExit("good bye");
#endif
  /*
   * If we reach this point,
   * something went horribly wrong
   */
}
