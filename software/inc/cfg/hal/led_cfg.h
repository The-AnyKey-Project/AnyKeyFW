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
 * led_cfg.h
 *
 *  Created on: 28.03.2021
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_LED_CFG_H_
#define INC_CFG_HAL_LED_CFG_H_

#define LED_NUMBER 4

#define LED_ANIMATION_THREAD_STACK     128
#define LED_ANIMATION_THREAD_PRIO      (NORMALPRIO)
#define LED_ANIMATION_MAIN_THREAD_P_MS 10

#define LED_TICKS_PER_MS (LED_PWM_FREQ / 1000)
#define LED_TICKS_PER_US (LED_TICKS_PER_MS / 1000)

#define LED_PWM_FREQ         20000000                    // 20MHz timer speed
#define LED_PWM_PERIOD       (LED_TICKS_PER_US * 5 / 4)  // 1,25us period
#define LED_PWM_OUTPUT_LINE  PAL_LINE(GPIOB, 8U)
#define LED_PWM_TIMER_DRIVER (&PWMD4)
#define LED_PWM_TIMER_CH     3
#define LED_PWM_DMA_TRIGGER  STM32_TIM_DIER_CC3DE

#define LED_DMA_STREAM   STM32_DMA_STREAM_ID(1, 5)
#define LED_DMA_CH       5
#define LED_DMA_IRQ_PRIO 1
#define LED_DMA_CH_PRIO  1

#define LED_WS2812B_ONE         (LED_TICKS_PER_US * 9 / 10)    // 0,9 us
#define LED_WS2812B_ZERO        (LED_TICKS_PER_US * 35 / 100)  // 0,35 us
#define LED_WS2812B_COLOR_ORDER 1  // Depend on WS2812 version -> 0 RGB, 1 GRB

#define LED_PWM_CHANNEL_CFG                               \
  {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},        \
      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},    \
      {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL}, \
      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},

#define LED_BIT_BUFFER_SIZE (LED_NUMBER + 2)  // use 2 additional LEDs to create 50us gap
#define LED_PWM_CCR         (LED_PWM_TIMER_DRIVER->tim->CCR[LED_PWM_TIMER_CH - 1])

#endif /* INC_CFG_HAL_LED_CFG_H_ */
