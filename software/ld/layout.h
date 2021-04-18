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

#ifndef LD_LAYOUT_H_
#define LD_LAYOUT_H_

#define LAYOUT_FLASH_START 0x08000000
#define LAYOUT_FLASH_SIZE  64k
#define LAYOUT_RAM_START   0x20000000
#define LAYOUT_RAM_SIZE    20k

#define LAYOUT_BOOTLOADER_FLASH_SIZE 0x5000
#define LAYOUT_BOOTLOADER_RAM_SIZE   0xC00

#ifdef USE_MAPLEMINI_BOOTLOADER
#define LAYOUT_FLASH_AVAILABLE (LAYOUT_FLASH_SIZE - LAYOUT_BOOTLOADER_FLASH_SIZE)
#define LAYOUT_FLASH_OFFSET    (LAYOUT_FLASH_START + LAYOUT_BOOTLOADER_FLASH_SIZE)
#define LAYOUT_RAM_AVAILABLE   (LAYOUT_RAM_SIZE - LAYOUT_BOOTLOADER_RAM_SIZE)
#define LAYOUT_RAM_OFFSET      (LAYOUT_RAM_START + LAYOUT_BOOTLOADER_RAM_SIZE)
#else
#define LAYOUT_FLASH_AVAILABLE (LAYOUT_FLASH_SIZE)
#define LAYOUT_FLASH_OFFSET    (LAYOUT_FLASH_START)
#define LAYOUT_RAM_AVAILABLE   (LAYOUT_RAM_SIZE)
#define LAYOUT_RAM_OFFSET      (LAYOUT_RAM_START)
#endif

#ifdef USE_DEBUG_BUILD
#ifdef USE_CMD_SHELL
#define LAYOUT_CODE_SIZE 48k
#else
#define LAYOUT_CODE_SIZE
#endif
#else
#ifdef USE_CMD_SHELL
#define LAYOUT_CODE_SIZE
#else
#define LAYOUT_CODE_SIZE
#endif
#endif

#define LAYOUT_FLASH1_START (LAYOUT_FLASH_OFFSET)
#define LAYOUT_FLASH1_SIZE  LAYOUT_CODE_SIZE
#define LAYOUT_FLASH2_START (LAYOUT_FLASH1_START + LAYOUT_FLASH1_SIZE)
#define LAYOUT_FLASH2_SIZE  (LAYOUT_FLASH_AVAILABLE - LAYOUT_FLASH1_SIZE)
#endif /* LD_LAYOUT_H_ */
