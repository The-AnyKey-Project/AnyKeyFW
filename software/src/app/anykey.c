/*
 * anykey.c
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
static_assert(ANYKEY_NUMBER_OF_KEYS == (KEYPAD_SW_COUNT), "Number of keys does not match number used keypad switches");
static_assert(ANYKEY_NUMBER_OF_KEYS == (GLCD_DISP_MAX),   "Number of keys does not match number used displays");

/*
 * Forward declarations of static functions
 */
static void             _anykey_init_module(void);

/*
 * Static variables
 */
static THD_WORKING_AREA(_anykey_key_stack, ANYKEY_KEY_THREAD_STACK);
static THD_WORKING_AREA(_anykey_cmd_stack, ANYKEY_CMD_THREAD_STACK);

/*
 * Global variables
 */

/*
 * Tasks
 */

static __attribute__((noreturn)) THD_FUNCTION(_anykey_key_thread, arg)
{
  (void)arg;


  chRegSetThreadName("anykey_key_thread");

//  while(true)
//  {
//TODO
//  }
}

static __attribute__((noreturn)) THD_FUNCTION(_anykey_cmd_thread, arg)
{
  (void)arg;


  chRegSetThreadName("anykey_cmd_thread");

//  while(true)
//  {
//TODO
//  }

}


/*
 * Static helper functions
 */

static void _anykey_init_module(void)
{
  chThdCreateStatic(_anykey_key_stack, sizeof(_anykey_key_stack),
                    ANYKEY_KEY_THREAD_PRIO, _anykey_key_thread, NULL);
  chThdCreateStatic(_anykey_cmd_stack, sizeof(_anykey_cmd_stack),
                    ANYKEY_CMD_THREAD_PRIO, _anykey_cmd_thread, NULL);
}


/*
 * Callback functions
 */

/*
 * Shell functions
 */

void anykey_loop_keys(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void anykey_show_layer(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void anykey_list_layers(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

void anykey_set_layer(BaseSequentialStream *chp, int argc, char *argv[])
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
  _anykey_init_module();

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