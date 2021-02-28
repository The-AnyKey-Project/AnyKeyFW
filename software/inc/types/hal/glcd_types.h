/*
 * glcd_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_TYPES_HAL_GLCD_TYPES_H_
#define INC_TYPES_HAL_GLCD_TYPES_H_

#include "u8g2.h"

typedef enum
{
  GLCD_DISP_1 = 0,
  GLCD_DISP_2,
  GLCD_DISP_3,
  GLCD_DISP_4,
  GLCD_DISP_5,
  GLCD_DISP_6,
  GLCD_DISP_7,
  GLCD_DISP_8,
  GLCD_DISP_9,
  GLCD_DISP_MAX
} glcd_display_id_t;

typedef struct
{
  uint8_t x_size;
  uint8_t y_size;
  uint8_t x_offset;
  uint8_t y_offset;
  uint8_t *content;
} glcd_display_buffer_t;

#endif /* INC_TYPES_HAL_GLCD_TYPES_H_ */
