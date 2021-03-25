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
 * keypad_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_CMD_KEYPAD_CMD_H_
#define INC_CMD_KEYPAD_CMD_H_

/*
 * Global definition of shell commands
 * for module keypad
 */
extern void keypad_loop_switches_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module kepad
 */
// clang-format off
#define KEYPAD_CMD_LIST \
    {"kp-loop-sw", keypad_loop_switches_sh}
// clang-format on

#endif /* INC_CMD_KEYPAD_CMD_H_ */
