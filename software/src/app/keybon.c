/*
 * keybon.c
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
#include "api/app/keybon.h"

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
static_assert(KEYBON_NUMBER_OF_KEYS == (KEYPAD_SW_COUNT), "Number of keys does not match number used keypad switches");
static_assert(KEYBON_NUMBER_OF_KEYS == (GLCD_DISP_MAX),   "Number of keys does not match number used displays");

/*
 * Forward declarations of static functions
 */
static void             _keybon_init_module(void);

/*
 * Static variables
 */
static THD_WORKING_AREA(_keybon_key_stack, KEYBON_KEY_THREAD_STACK);
static THD_WORKING_AREA(_keybon_cmd_stack, KEYBON_CMD_THREAD_STACK);

/*
 * Global variables
 */

/*
 * Tasks
 */

static __attribute__((noreturn)) THD_FUNCTION(_keybon_key_thread, arg)
{
  (void)arg;


  chRegSetThreadName("keybon_key_thread");

//  while(true)
//  {
//
//  }
}

static __attribute__((noreturn)) THD_FUNCTION(_keybon_cmd_thread, arg)
{
  (void)arg;


  chRegSetThreadName("keybon_cmd_thread");

//  while(true)
//  {
//
//  }

}


/*
 * Static helper functions
 */

static void _keybon_init_module(void)
{
  chThdCreateStatic(_keybon_key_stack, sizeof(_keybon_key_stack),
                    KEYBON_KEY_THREAD_PRIO, _keybon_key_thread, NULL);
  chThdCreateStatic(_keybon_cmd_stack, sizeof(_keybon_cmd_stack),
                    KEYBON_CMD_THREAD_PRIO, _keybon_cmd_thread, NULL);
}


/*
 * Callback functions
 */

/*
 * Shell functions
 */

void keybon_loop_keys(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void keybon_show_layer(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void keybon_list_layers(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void keybon_set_layer(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

/*
 * API functions
 */

void keybon_init(void)
{
  /*
   * Project specific hal initialization
   */
//  glcd_init();
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
  _keybon_init_module();

  /*
   * Start shell handling,
   * should never return
   */
#if defined(USE_CMD_SHELL)
  cmd_shell_loop();
#else
  while(true) ;
#endif
  /*
   * If we reach this point,
   * something went horribly wrong
   */
}
