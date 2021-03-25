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
 * glcd.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_API_GLCD_H_
#define INC_API_GLCD_H_

#include "cfg/hal/glcd_cfg.h"
#include "types/hal/glcd_types.h"

extern void glcd_init(void);
extern void glcd_set_displays(uint32_t *buffer);
extern uint8_t glcd_set_contrast(glcd_display_id_t display, uint8_t value);
extern uint8_t glcd_get_contrast(glcd_display_id_t display);

#endif /* INC_API_GLCD_H_ */
