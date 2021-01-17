/*
 * keypad_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_TYPES_HAL_KEYPAD_TYPES_H_
#define INC_TYPES_HAL_KEYPAD_TYPES_H_

typedef enum {
  KEYPAD_SW_ID_MIN = 0,
  KEYPAD_SW_ID_SW01 = KEYPAD_SW_ID_MIN,
  KEYPAD_SW_ID_SW02,
  KEYPAD_SW_ID_SW03,
  KEYPAD_SW_ID_SW04,
  KEYPAD_SW_ID_SW05,
  KEYPAD_SW_ID_SW06,
  KEYPAD_SW_ID_SW07,
  KEYPAD_SW_ID_SW08,
  KEYPAD_SW_ID_SW09,
  KEYPAD_SW_ID_MAX,
}keypad_sw_id_t;

typedef struct {
  uint32_t line;
  uint32_t delay;
  enum {
    KEYPAD_SW_STATE_INIT = 0,
    KEYPAD_SW_STATE_PRESS
  }state;
}keypad_sw_t;

#endif /* INC_TYPES_HAL_KEYPAD_TYPES_H_ */
