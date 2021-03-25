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
 * flash_storage_cmd.h
 *
 *  Created on: 04.03.2021
 *      Author: matthiasb85
 */

#ifndef INC_CMD_HAL_FLASH_STORAGE_CMD_H_
#define INC_CMD_HAL_FLASH_STORAGE_CMD_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module flash storage
 */
extern void flash_storage_info_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void flash_storage_write_default_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module flash_storage
 */
// clang-format off
#define FLASH_STORAGE_CMD_LIST \
            {"fs-info",   flash_storage_info_sh}, \
            {"fs-write-default",   flash_storage_write_default_sh} \
// clang-format on
#endif

#endif /* INC_CMD_HAL_FLASH_STORAGE_CMD_H_ */
