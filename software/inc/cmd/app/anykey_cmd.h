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
 * anykey_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_CMD_ANYKEY_CMD_H_
#define INC_CMD_ANYKEY_CMD_H_

/*
 * Global definition of shell commands
 * for module anykey
 */
extern void anykey_show_actions_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_show_layer_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_list_layers_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_set_layer_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module anykey
 */
// clang-format off
#define ANYKEY_CMD_LIST \
            {"ak-show-actions", anykey_show_actions_sh}, \
            {"ak-show-layer",  anykey_show_layer_sh}, \
            {"ak-list-layers", anykey_list_layers_sh}, \
            {"ak-set-layer",   anykey_set_layer_sh}
// clang-format on
#endif /* INC_CMD_ANYKEY_CMD_H_ */
