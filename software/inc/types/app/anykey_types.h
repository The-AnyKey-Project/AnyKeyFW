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
  glcd_display_buffer_t* displays[ANYKEY_NUMBER_OF_KEYS];
  anykey_action_list_t* key_actions_press[ANYKEY_NUMBER_OF_KEYS];
  anykey_action_list_t* key_actions_release[ANYKEY_NUMBER_OF_KEYS];
} anykey_layer_t;

typedef struct
{
  anykey_action_t action;
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
