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
extern void glcd_set_displays(glcd_display_buffer_t * buffer);

#endif /* INC_API_GLCD_H_ */
