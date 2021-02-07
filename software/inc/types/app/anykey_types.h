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
  ANYKEY_ACTION_KEY = 0,
  ANYKEY_ACTION_KEY_EXT,
  ANYKEY_ACTION_NEXT_LAYER,
  ANYKEY_ACTION_PREV_LAYER,
  ANYKEY_ACTION_ADJUST_CONTRAST
} anykey_action_t;

typedef struct
{
  uint8_t length;
  uint8_t* actions;
} anykey_action_list_t;

typedef struct _anykey_layer_t
{
  struct _anykey_layer_t* next;
  struct _anykey_layer_t* prev;
  char* name;
  glcd_display_buffer_t displays[ANYKEY_NUMBER_OF_KEYS];
  anykey_action_list_t* key_actions[ANYKEY_NUMBER_OF_KEYS];
} anykey_layer_t;

#endif /* INC_TYPES_APP_ANYKEY_TYPES_H_ */
