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
 * keypad.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_API_KEYPAD_H_
#define INC_API_KEYPAD_H_

#include "cfg/hal/keypad_cfg.h"
#include "types/hal/keypad_types.h"

extern event_source_t keypad_event_handle;

extern void keypad_init(void);
extern void keypad_get_sw_events(keypad_event_t* dest);

#endif /* INC_API_KEYPAD_H_ */
