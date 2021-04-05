/*
 * This file is part of The AnyKey Project  https://github.com/The-AnyKey-Project
 *
 * Copyright (c) 2021 Matthias Beckert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * led_types.h
 *
 *  Created on: 28.03.2021
 *      Author: matthiasb85
 */

#ifndef INC_TYPES_HAL_LED_TYPES_H_
#define INC_TYPES_HAL_LED_TYPES_H_

typedef enum
{
  LED_RGB = 0,
  LED_HSV,
} led_type_t;

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} led_rgb_t;

typedef struct
{
  uint8_t h;
  uint8_t s;
  uint8_t v;
} led_hsv_t;

typedef struct
{
  led_type_t type;
  union
  {
    led_rgb_t rgb;
    led_hsv_t hsv;
  };
} led_t;

#if LED_WS2812B_ZERO > 255 || LED_WS2812B_ONE > 255
#warning "Try to avoid WS_BIT_ZERO > 255 || WS_BIT_ONE > 255 in order to decrease RAM usage!"
typedef uint16_t led_dma_bit_size_t;
#define led_dma_transfer_t STM32_DMA_CR_MSIZE_HWORD
#else
typedef uint8_t led_dma_bit_size_t;
#define led_dma_transfer_t STM32_DMA_CR_MSIZE_BYTE
#endif

typedef struct
{
#if LED_WS2812B_COLOR_ORDER == 0
  led_dma_bit_size_t r[8];
  led_dma_bit_size_t g[8];
  led_dma_bit_size_t b[8];
#elif LED_WS2812B_COLOR_ORDER == 1
  led_dma_bit_size_t g[8];
  led_dma_bit_size_t r[8];
  led_dma_bit_size_t b[8];
#endif
} led_bitfield_t;

typedef enum
{
  LED_ANIMATION_NONE = 0,
  LED_ANIMATION_STATIC,
  LED_ANIMATION_PULSE,
  LED_ANIMATION_RAINBOW
} led_anymation_type_t;

typedef struct
{
  led_rgb_t color;
} led_animation_static_t;

typedef struct
{
  led_hsv_t color;
  uint32_t period;
} led_animation_pulse_t;

typedef struct
{
  uint32_t period;
} led_animation_rainbow_t;

typedef struct
{
  led_anymation_type_t type;
  union
  {
    led_animation_static_t static_color;
    led_animation_pulse_t pulse;
    led_animation_rainbow_t rainbow;
  };
} led_animation_t;

#endif /* INC_TYPES_HAL_LED_TYPES_H_ */
