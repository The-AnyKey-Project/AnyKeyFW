/*
 * cmd_shell.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

/*
 * Include ChibiOS & HAL
 */
#include "ch.h"
#include "hal.h"
#include "shell.h"
#include "chprintf.h"

/*
 * Includes module API, types & config
 */
#include "api/app/cmd_shell.h"

/*
 * Include dependencies
 */
#include "api/hal/usb.h"
#include "cmd/app/keybon_cmd.h"
#include "cmd/hal/glcd_cmd.h"
#include "cmd/hal/keypad_cmd.h"
#include "cmd/hal/usb_cmd.h"

/*
 * Forward declarations of static functions
 */
static void     _cmd_shell_init_hal     (void);
static void     _cmd_shell_init_module  (void);

/*
 * Static variables
 */
static const ShellCommand _cmd_shell_cmds[] = {
  KEYBON_CMD_LIST
  GLCD_CMD_LIST
  KEYPAD_CMD_LIST
  USB_CMD_LIST
  {NULL, NULL}
};

static const ShellConfig _cmd_shell_cfg = {
  (BaseSequentialStream *)&USB_SERIAL_DRIVER_HANDLE,
  _cmd_shell_cmds
};

/*
 * Global variables
 */

/*
 * Static helper functions
 */
static void _cmd_shell_init_hal(void)
{

}

static void _cmd_shell_init_module(void)
{
  /*
   * Shell manager initialization.
   */
  shellInit();
}

/*
 * API functions
 */
void cmd_shell_init(void)
{
  _cmd_shell_init_hal();
  _cmd_shell_init_module();
}

void cmd_shell_loop(void)
{

  /*
   * Use main() thread to spawn shells.
   */
  while (true) {
    if (USB_SERIAL_DRIVER_HANDLE.config->usbp->state == USB_ACTIVE)
    {
      thread_t *shelltp = chThdCreateFromHeap(NULL,
          THD_WORKING_AREA_SIZE(CMD_SHELL_WA_SIZE),
          "shell", CMD_SHELL_PRIO,
          shellThread, (void *)&_cmd_shell_cfg);

      chThdWait(shelltp);               /* Waiting termination.             */
    }
    chThdSleepMilliseconds(1000);
  }
}

