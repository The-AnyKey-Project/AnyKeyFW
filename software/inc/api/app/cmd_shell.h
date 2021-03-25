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
 * cmd_shell.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_API_SHELL_H_
#define INC_API_SHELL_H_

#include "cfg/app/cmd_shell_cfg.h"
#include "types/app/cmd_shell_types.h"

extern void cmd_shell_init(void);
extern void cmd_shell_loop(void);

#endif /* INC_API_SHELL_H_ */
