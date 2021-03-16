/*
 * anykey_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_TYPES_APP_ANYKEY_TYPES_H_
#define INC_TYPES_APP_ANYKEY_TYPES_H_

#include "api/hal/glcd.h"

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

#endif /* INC_TYPES_APP_ANYKEY_TYPES_H_ */
