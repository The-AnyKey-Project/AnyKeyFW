/*
 * glcd.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_API_GLCD_H_
#define INC_API_GLCD_H_

#include "cfg/hal/glcd_cfg.h"
#include "types/hal/glcd_types.h"

extern void glcd_init(void);
extern void glcd_set_displays(uint32_t *buffer);
extern uint8_t glcd_set_contrast(glcd_display_id_t display, uint8_t value);
extern uint8_t glcd_get_contrast(glcd_display_id_t display);

#endif /* INC_API_GLCD_H_ */
