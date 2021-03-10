/*
 * flash_storage_types.h
 *
 *  Created on: 04.03.2021
 *      Author: matti
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
} flash_storage_header_t;

typedef struct
{
  flash_storage_header_t flash_header;
  anykey_layer_t layer_header;
  uint8_t name[FLASH_STORAGE_DEFCONFIG_NAME_LENGTH];
  struct
  {
    glcd_display_header_t header;
    uint8_t content[FLASH_STORAGE_DEFCONFIG_DB_LENGTH];
  } db[ANYKEY_NUMBER_OF_KEYS];
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } kp_1;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } kp_2;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } kp_3;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kp_4;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kp_5;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kp_6;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kp_7;
  struct
  {
    uint8_t length;
    anykey_action_contrast_t act_1;
  } kp_8;
  struct
  {
    uint8_t length;
    anykey_action_contrast_t act_1;
  } kp_9;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } kr_1;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } kr_2;
  struct
  {
    uint8_t length;
    anykey_action_key_t act_1;
    anykey_action_key_t act_2;
  } kr_3;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kr_4;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kr_5;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kr_6;
  struct
  {
    uint8_t length;
    anykey_action_keyext_t act_1;
  } kr_7;
} flash_storage_default_layer_t;

#define FLASH_STORAGE_DB_CONTENT(x)                                                        \
  .header = {.x_size = FLASH_STORAGE_DEFCONFIG_DB_X_SIZE,                                  \
             .y_size = FLASH_STORAGE_DEFCONFIG_DB_Y_SIZE,                                  \
             .x_offset = ((GLCD_DISPLAY_WIDTH - FLASH_STORAGE_DEFCONFIG_DB_X_SIZE) / 2),   \
             .y_offset = ((GLCD_DISPLAY_HEIGHT - FLASH_STORAGE_DEFCONFIG_DB_Y_SIZE) / 2)}, \
  .content = {x}
#define FLASH_STORAGE_KEY_CONTENT(x, y)       .action = x, .key = y
#define FLASH_STORAGE_KEYEXT_CONTENT(x, y, z) .action = x, .report_id = y, .key = z
#define FLASH_STORAGE_CONTRAST_CONTENT(x, y)  .action = x, .adjust = y

#endif /* INC_TYPES_HAL_FLASH_STORAGE_TYPES_H_ */
