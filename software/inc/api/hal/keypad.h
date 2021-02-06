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

extern event_source_t keypad_event_handle;

extern void keypad_init(void);
extern void keypad_get_sw_events(keypad_event_t* dest);

#endif /* INC_API_KEYPAD_H_ */
