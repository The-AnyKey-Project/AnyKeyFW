/*
 * anykey_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_TYPES_APP_ANYKEY_TYPES_H_
#define INC_TYPES_APP_ANYKEY_TYPES_H_

#include "api/hal/glcd.h"
#include "api/hal/usb.h"

/*
 * Layer & action type definitions
 */
typedef enum
{
  ANYKEY_ACTION_KEY_PRESS = 0,
  ANYKEY_ACTION_KEYEXT_PRESS,
  ANYKEY_ACTION_KEY_RELEASE,
  ANYKEY_ACTION_KEYEXT_RELEASE,
  ANYKEY_ACTION_NEXT_LAYER,
  ANYKEY_ACTION_PREV_LAYER,
  ANYKEY_ACTION_ADJUST_CONTRAST
} __attribute__((packed)) anykey_action_t;

typedef struct
{
  uint8_t length;     // length of action list
  uint8_t actions[];  // flash storage idx of first entry
} anykey_action_list_t;

typedef struct _anykey_layer_t
{
  uint32_t next_idx;                            // flash storage idx of next layer
  uint32_t prev_idx;                            // flash storage idx of prev layer
  uint32_t name_idx;                            // flash storage idx of name
  uint32_t display_idx[ANYKEY_NUMBER_OF_KEYS];  // array of flash storage idx for display buffers
  uint32_t key_action_press_idx[ANYKEY_NUMBER_OF_KEYS];  // array of flash storage idx for key press
                                                         // actions
  uint32_t key_action_release_idx[ANYKEY_NUMBER_OF_KEYS];  // array of flash storage idx for key
                                                           // release actions
} anykey_layer_t;

typedef struct
{
  anykey_action_t action;
  uint8_t mods;
  uint8_t key;
} anykey_action_key_t;

typedef struct
{
  anykey_action_t action;
  uint8_t report_id;
  uint16_t key;
} anykey_action_keyext_t;

typedef struct
{
  anykey_action_t action;
} anykey_action_layer_t;

typedef struct
{
  anykey_action_t action;
  int8_t adjust;
} anykey_action_contrast_t;

/*
 * USB command definitions
 */
typedef enum
{
  ANYKEY_CMD_SET_LAYER = 0,
  ANYKEY_CMD_GET_LAYER,
  ANYKEY_CMD_SET_CONTRAST,
  ANYKEY_CMD_GET_CONTRAST,
  ANYKEY_CMD_GET_FLASH_INFO,
  ANYKEY_CMD_SET_FALSH,
  ANYKEY_CMD_GET_FALSH,
} __attribute__((packed)) anykey_cmd_t;

/*
 * Request typed definitions
 */
typedef struct
{
  anykey_cmd_t cmd;
  uint8_t name[USB_HID_RAW_EPSIZE - sizeof(anykey_cmd_t)];
} __attribute__((packed)) anykey_cmd_set_layer_req_t;

typedef struct
{
  anykey_cmd_t cmd;
} __attribute__((packed)) anykey_cmd_get_layer_req_t;

typedef struct
{
  anykey_cmd_t cmd;
  glcd_display_id_t display;
  int8_t contrast[GLCD_DISP_MAX];
} __attribute__((packed)) anykey_cmd_set_contrast_req_t;

typedef struct
{
  anykey_cmd_t cmd;
  glcd_display_id_t display;
} __attribute__((packed)) anykey_cmd_get_contrast_req_t;

typedef struct
{
  anykey_cmd_t cmd;
} __attribute__((packed)) anykey_cmd_get_flash_info_req_t;

typedef struct
{
  anykey_cmd_t cmd;
  struct
  {
    uint16_t block_cnt : 15;
    uint16_t final_block : 1;
  };
  uint16_t sector;
  uint16_t block_size;
  uint8_t buffer[USB_HID_RAW_EPSIZE - sizeof(anykey_cmd_t) - 2 * sizeof(uint16_t)];
} __attribute__((packed)) anykey_cmd_set_flash_req_t;

typedef struct
{
  anykey_cmd_t cmd;
  uint32_t sector;
} __attribute__((packed)) anykey_cmd_get_flash_req_t;

typedef union
{
  struct
  {
    anykey_cmd_t cmd;
    uint8_t padding[USB_HID_RAW_EPSIZE - sizeof(anykey_cmd_t)];
  } __attribute__((packed)) raw;
  anykey_cmd_set_layer_req_t set_layer;
  anykey_cmd_set_contrast_req_t set_contrast;
  anykey_cmd_get_layer_req_t get_layer;
  anykey_cmd_get_contrast_req_t get_contrast;
  anykey_cmd_get_flash_info_req_t get_flash_info;
  anykey_cmd_set_flash_req_t set_flash;
  anykey_cmd_set_flash_req_t get_flash;
} anykey_cmd_req_t;

/*
 * Response typed definitions
 */
typedef struct
{
  anykey_cmd_t cmd;
  uint8_t name[USB_HID_RAW_EPSIZE - sizeof(anykey_cmd_t)];
} __attribute__((packed)) anykey_cmd_get_layer_resp_t;

typedef struct
{
  anykey_cmd_t cmd;
  int8_t contrast[GLCD_DISP_MAX];
} __attribute__((packed)) anykey_cmd_get_contrast_resp_t;

typedef struct
{
  anykey_cmd_t cmd;
  uint32_t flash_size;
  uint32_t sector_size;
} __attribute__((packed)) anykey_cmd_get_flash_info_resp_t;

typedef struct
{
  anykey_cmd_t cmd;
  struct
  {
    uint16_t block_cnt : 15;
    uint16_t final_block : 1;
  };
} __attribute__((packed)) anykey_cmd_set_flash_resp_t;

typedef struct
{
  anykey_cmd_t cmd;
  uint16_t block_size;
  struct
  {
    uint16_t block_cnt : 15;
    uint16_t final_block : 1;
  };
  uint8_t buffer[USB_HID_RAW_EPSIZE - sizeof(anykey_cmd_t) - sizeof(uint16_t)];
} __attribute__((packed)) anykey_cmd_get_flash_resp_t;

typedef union
{
  struct
  {
    anykey_cmd_t cmd;
    uint8_t padding[USB_HID_RAW_EPSIZE - sizeof(anykey_cmd_t)];
  } __attribute__((packed)) raw;
  anykey_cmd_get_layer_resp_t get_layer;
  anykey_cmd_get_contrast_resp_t get_contrast;
  anykey_cmd_get_flash_info_resp_t get_flash_info;
  anykey_cmd_set_flash_resp_t set_flash;
  anykey_cmd_get_flash_resp_t get_flash;
} anykey_cmd_resp_t;

#endif /* INC_TYPES_APP_ANYKEY_TYPES_H_ */
