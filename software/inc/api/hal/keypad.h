/*
 * keypad.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_API_KEYPAD_H_
#define INC_API_KEYPAD_H_

#include "cfg/hal/keypad_cfg.h"
#include "types/hal/keypad_types.h"

extern void keypad_init(void);
extern uint32_t keypad_get_sw_states(void);

#endif /* INC_API_KEYPAD_H_ */
