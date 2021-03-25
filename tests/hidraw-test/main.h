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
 * main.h
 *
 *  Created on: 21.03.2021
 *      Author: matthiasb85
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

/* AnyKey */
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
  uint8_t q;
} cli_args_t;

typedef void (*action_callback)(int fd, uint8_t *buf, cli_args_t *args);

#endif /* MAIN_H_ */
