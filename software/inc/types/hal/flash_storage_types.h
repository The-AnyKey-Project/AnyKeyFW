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
 * flash_storage_types.h
 *
 *  Created on: 04.03.2021
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_
#define INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_

#include "api/app/anykey.h"
#include "api/hal/glcd.h"

typedef uint32_t crc_t;

typedef struct
{
  crc_t crc;
  uint32_t version;
  uint32_t initial_layer_idx;
  uint32_t first_layer_idx;
  uint8_t display_contrast[ANYKEY_NUMBER_OF_KEYS];
} flash_storage_header_t;

typedef struct
{
  flash_storage_header_t flash_header;
  anykey_layer_t l1_header;
  anykey_layer_t l2_header;
  uint8_t l1_name[FLASH_STORAGE_DEFCONFIG_NAME_LENGTH];
  uint8_t l2_name[FLASH_STORAGE_DEFCONFIG_NAME_LENGTH];
  struct
  {
    glcd_display_header_t header;
    uint8_t content[FLASH_STORAGE_DEFCONFIG_DB_LENGTH];
  } db[ANYKEY_NUMBER_OF_KEYS];
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
  } __attribute__((packed)) kp_0;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
  } __attribute__((packed)) kp_1;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
  } __attribute__((packed)) kp_2;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kp_3;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kp_4;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kp_5;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kp_6;
  struct
  {
    uint8_t length;
    anykey_action_contrast_t act_1;
  } __attribute__((packed)) kp_7;
  struct
  {
    uint8_t length;
    anykey_action_contrast_t act_1;
  } __attribute__((packed)) kp_8;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } __attribute__((packed)) kr_0;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } __attribute__((packed)) kr_1;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } __attribute__((packed)) kr_2;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kr_3;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kr_4;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kr_5;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } __attribute__((packed)) kr_6;
} __attribute__((packed)) flash_storage_default_layer_t;

#define FLASH_STORAGE_DB_CONTENT(x)                                                        \
  .header = {.x_size = FLASH_STORAGE_DEFCONFIG_DB_X_SIZE,                                  \
             .y_size = FLASH_STORAGE_DEFCONFIG_DB_Y_SIZE,                                  \
             .x_offset = ((GLCD_DISPLAY_WIDTH - FLASH_STORAGE_DEFCONFIG_DB_X_SIZE) / 2),   \
             .y_offset = ((GLCD_DISPLAY_HEIGHT - FLASH_STORAGE_DEFCONFIG_DB_Y_SIZE) / 2)}, \
  .content = {x}
#define FLASH_STORAGE_KEY_CONTENT(x, y, z) \
  {                                        \
    .action = x, .mods = y, .key = z       \
  }
#define FLASH_STORAGE_KEYEXT_CONTENT(x, y, z) \
  {                                           \
    .action = x, .report_id = y, .key = z     \
  }
#define FLASH_STORAGE_CONTRAST_CONTENT(x, y) \
  {                                          \
    .action = x, .adjust = y                 \
  }

#define FLASH_STORAGE_KEY_PRESS(y, z)   FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, y, z)
#define FLASH_STORAGE_KEY_RELEASE(y, z) FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, y, z)
#define FLASH_STORAGE_KEYEXT_PRESS(y, z) \
  FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_PRESS, y, z)
#define FLASH_STORAGE_KEYEXT_RELEASE(y, z) \
  FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_RELEASE, y, z)
#define FLASH_STORAGE_CONTRAST_PRESS(y) \
  FLASH_STORAGE_CONTRAST_CONTENT(ANYKEY_ACTION_ADJUST_CONTRAST, y)

#endif /* INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_ */
