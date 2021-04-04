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
 * cmd_shell.c
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
#include "shell.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/app/cmd_shell.h"

/*
 * Include dependencies
 */
#include "api/hal/usb.h"
#include "cmd/app/anykey_cmd.h"
#include "cmd/hal/flash_storage_cmd.h"
#include "cmd/hal/glcd_cmd.h"
#include "cmd/hal/keypad_cmd.h"
#include "cmd/hal/led_cmd.h"
#include "cmd/hal/usb_cmd.h"

#if defined(USE_CMD_SHELL)
/*
 * Forward declarations of static functions
 */
static void _cmd_shell_init_hal(void);
static void _cmd_shell_init_module(void);

/*
 * Static variables
 */
// clang-format off
static const ShellCommand _cmd_shell_cmds[] = {
  ANYKEY_CMD_LIST,
  FLASH_STORAGE_CMD_LIST,
  GLCD_CMD_LIST,
  KEYPAD_CMD_LIST,
  LED_CMD_LIST,
  USB_CMD_LIST,
  {NULL, NULL}
};

static const ShellConfig _cmd_shell_cfg = {
  (BaseSequentialStream *)&USB_CDC_DRIVER_HANDLE,
  _cmd_shell_cmds
};
// clang-format on

/*
 * Global variables
 */

/*
 * Static helper functions
 */
static void _cmd_shell_init_hal(void) {}

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
  while (true)
  {
    if (USB_CDC_DRIVER_HANDLE.config->usbp->state == USB_ACTIVE)
    {
      thread_t *shelltp =
          chThdCreateFromHeap(NULL, THD_WORKING_AREA_SIZE(CMD_SHELL_WA_SIZE), "shell",
                              CMD_SHELL_PRIO, shellThread, (void *)&_cmd_shell_cfg);

      chThdWait(shelltp); /* Waiting termination.             */
    }
    chThdSleepMilliseconds(1000);
  }
}
#endif
