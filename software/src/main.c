/*
 * main.c
 *
 *  Created on: 06.01.2021
 *      Author: matti
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
int main(void) {

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
