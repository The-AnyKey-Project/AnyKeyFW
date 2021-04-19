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
 * layout.h
 *
 *  Created on: 18.04.2021
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_LINKER_LAYOUT_CFG_H_
#define INC_CFG_HAL_LINKER_LAYOUT_CFG_H_

#define LINKER_LAYOUT_FLASH_START 0x08000000
#define LINKER_LAYOUT_FLASH_SIZE  (64 * 1024)
#define LINKER_LAYOUT_RAM_START   0x20000000
#define LINKER_LAYOUT_RAM_SIZE    (20 * 1024)

#define LINKER_LAYOUT_BOOTLOADER_FLASH_SIZE 0x2000
#define LINKER_LAYOUT_BOOTLOADER_RAM_SIZE   0xC00

#ifdef USE_MAPLEMINI_BOOTLOADER
#define LINKER_LAYOUT_FLASH_AVAILABLE (LINKER_LAYOUT_FLASH_SIZE - LINKER_LAYOUT_BOOTLOADER_FLASH_SIZE)
#define LINKER_LAYOUT_FLASH_OFFSET    (LINKER_LAYOUT_FLASH_START + LINKER_LAYOUT_BOOTLOADER_FLASH_SIZE)
#define LINKER_LAYOUT_RAM0_SIZE       (LINKER_LAYOUT_RAM_SIZE - LINKER_LAYOUT_BOOTLOADER_RAM_SIZE)
#define LINKER_LAYOUT_RAM0_START      (LINKER_LAYOUT_RAM_START + LINKER_LAYOUT_BOOTLOADER_RAM_SIZE)
#else
#define LINKER_LAYOUT_FLASH_AVAILABLE (LINKER_LAYOUT_FLASH_SIZE)
#define LINKER_LAYOUT_FLASH_OFFSET    (LINKER_LAYOUT_FLASH_START)
#define LINKER_LAYOUT_RAM0_SIZE       (LINKER_LAYOUT_RAM_SIZE)
#define LINKER_LAYOUT_RAM0_START      (LINKER_LAYOUT_RAM_START)
#endif

#ifdef USE_DEBUG_BUILD
#ifdef USE_CMD_SHELL
#define LINKER_LAYOUT_CODE_SIZE (48 * 1024)
#else
#define LINKER_LAYOUT_CODE_SIZE (29 * 1024)
#endif
#else
#ifdef USE_CMD_SHELL
#define LINKER_LAYOUT_CODE_SIZE (37 * 1024)
#else
#define LINKER_LAYOUT_CODE_SIZE (24 * 1024)
#endif
#endif

#define LINKER_LAYOUT_FLASH0_START (LINKER_LAYOUT_FLASH_OFFSET)
#define LINKER_LAYOUT_FLASH0_SIZE  LINKER_LAYOUT_CODE_SIZE
#define LINKER_LAYOUT_FLASH1_START (LINKER_LAYOUT_FLASH0_START + LINKER_LAYOUT_FLASH0_SIZE)
#define LINKER_LAYOUT_FLASH1_SIZE  (LINKER_LAYOUT_FLASH_AVAILABLE - LINKER_LAYOUT_FLASH0_SIZE)
#endif /* INC_CFG_HAL_LINKER_LAYOUT_CFG_H_ */
