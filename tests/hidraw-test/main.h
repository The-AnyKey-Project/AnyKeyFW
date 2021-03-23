/*
 * main.h
 *
 *  Created on: 21.03.2021
 *      Author: matti
 */

#ifndef MAIN_H_
#define MAIN_H_

/* Linux */
#include <linux/hidraw.h>
#include <linux/input.h>
#include <linux/types.h>

/* Unix */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* C */
#include <argp.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* anykey */
#include "api/app/anykey.h"

typedef struct
{
  char *D;
  anykey_action_t C;
  char *l;
  glcd_display_id_t d;
  uint8_t c;
  char *f;
  uint8_t v;
} cli_args_t;

typedef void (*action_callback)(int fd, uint8_t *buf, cli_args_t *args);

#endif /* MAIN_H_ */
