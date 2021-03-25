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
 * keypad_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_HAL_KEYPAD_TYPES_H_
#define INC_TYPES_HAL_KEYPAD_TYPES_H_

typedef enum
{
  KEYPAD_SW_ID_MIN = 0,
  KEYPAD_SW_ID_SW01 = KEYPAD_SW_ID_MIN,
  KEYPAD_SW_ID_SW02,
  KEYPAD_SW_ID_SW03,
  KEYPAD_SW_ID_SW04,
  KEYPAD_SW_ID_SW05,
  KEYPAD_SW_ID_SW06,
  KEYPAD_SW_ID_SW07,
  KEYPAD_SW_ID_SW08,
  KEYPAD_SW_ID_SW09,
  KEYPAD_SW_ID_MAX,
} keypad_sw_id_t;
#define KEYPAD_SW_COUNT (KEYPAD_SW_ID_MAX - KEYPAD_SW_ID_MIN)

typedef enum
{
  KEYPAD_EVENT_NONE = 0,
  KEYPAD_EVENT_PRESS,
  KEYPAD_EVENT_RELEASE
} keypad_event_t;

typedef struct
{
  uint32_t line;
  uint32_t delay;
  enum
  {
    KEYPAD_SW_STATE_INIT = 0,
    KEYPAD_SW_STATE_PRESS
  } state;
} keypad_sw_t;

#endif /* INC_TYPES_HAL_KEYPAD_TYPES_H_ */
