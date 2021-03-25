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
 * anykey_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_CFG_APP_ANYKEY_CFG_H_
#define INC_CFG_APP_ANYKEY_CFG_H_

#define ANYKEY_NUMBER_OF_KEYS 9

#define ANYKEY_KEY_THREAD_STACK 256
#define ANYKEY_KEY_THREAD_PRIO  (NORMALPRIO - 2)

#define ANYKEY_CMD_THREAD_STACK 256
#define ANYKEY_CMD_THREAD_PRIO  (NORMALPRIO - 1)

#endif /* INC_CFG_APP_ANYKEY_CFG_H_ */
