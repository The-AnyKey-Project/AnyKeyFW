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
 * glcd_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_CMD_GLCD_CMD_H_
#define INC_CMD_GLCD_CMD_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module glcd
 */
extern void glcd_set_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void glcd_get_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void glcd_reload_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module glcd
 */
// clang-format off
#define GLCD_CMD_LIST \
    {"glcd-set-contrast",    glcd_set_contrast_sh}, \
    {"glcd-get-contrast",    glcd_get_contrast_sh}, \
    {"glcd-reload-contrast", glcd_reload_contrast_sh}
// clang-format on
#endif

#endif /* INC_CMD_GLCD_CMD_H_ */
