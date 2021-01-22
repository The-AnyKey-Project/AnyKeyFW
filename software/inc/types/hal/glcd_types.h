/*
 * glcd_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_TYPES_HAL_GLCD_TYPES_H_
#define INC_TYPES_HAL_GLCD_TYPES_H_

#include "u8g2.h"

typedef enum {
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
}glcd_display_id_t;

typedef uint8_t (*glcd_spi_cb_t) (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
typedef uint8_t (*glcd_gpio_cb_t)(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

typedef struct {
  u8g2_t                handle;
  uint32_t              cs_line;
  glcd_spi_cb_t         spi_cb;
  glcd_gpio_cb_t        gpio_cb;
}glcd_display_t;

typedef struct {
  uint8_t content[GLCD_DISPLAY_BUFFER];
}glcd_display_buffer_t;

#endif /* INC_TYPES_HAL_GLCD_TYPES_H_ */
