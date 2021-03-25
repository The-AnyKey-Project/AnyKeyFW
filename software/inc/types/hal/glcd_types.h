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
 * glcd_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_HAL_GLCD_TYPES_H_
#define INC_TYPES_HAL_GLCD_TYPES_H_

#include "u8g2.h"

typedef enum
{
  GLCD_DISP_1 = 0,
  GLCD_DISP_2,
  GLCD_DISP_3,
  GLCD_DISP_4,
  GLCD_DISP_5,
  GLCD_DISP_6,
  GLCD_DISP_7,
  GLCD_DISP_8,
  GLCD_DISP_9,
  GLCD_DISP_MAX
} __attribute__((packed)) glcd_display_id_t;

typedef struct
{
  uint8_t x_size;
  uint8_t y_size;
  uint8_t x_offset;
  uint8_t y_offset;
} glcd_display_header_t;
typedef struct
{
  glcd_display_header_t header;
  uint8_t content[];
} glcd_display_buffer_t;

#endif /* INC_TYPES_HAL_GLCD_TYPES_H_ */
