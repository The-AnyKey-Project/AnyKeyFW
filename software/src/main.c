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
 * main.c
 *
 *  Created on: 06.01.2021
 *      Author: matthiasb85
 *
 *      based on ChibiOS example
 */

/*
 * ChibiOS specific includes
 */
#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

/*
 * Project specific API includes
 */
#include "api/app/anykey.h"

/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Project specific application initialization,
   * does not return since it also starts shell
   * thread handling
   */
  anykey_init();

  /*
   * HIC SVNT DRACONES
   */
}
