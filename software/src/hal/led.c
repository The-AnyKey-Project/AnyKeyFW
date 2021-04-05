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
 * led.c
 *
 *  Created on: 28.03.2021
 *      Author: matthiasb85
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/hal/led.h"

/*
 * Include dependencies
 */
#include <string.h>
#include <stdlib.h>
#include "api/app/anykey.h"

/*
 * Forward declarations of static functions
 */
static void _led_init_hal(void);
static void _led_init_module(void);
static void _led_set_led_bitfield(led_rgb_t* src, uint16_t id);
static void _led_set_led_bitfield_all(led_rgb_t* src);
static void _led_hsv_to_rbg(led_hsv_t* hsv, led_rgb_t* rgb);

/*
 * Static variables
 */
static THD_WORKING_AREA(_led_animation_stack, LED_ANIMATION_THREAD_STACK);
static PWMConfig _led_pwm_pwmd_cfg = {
    LED_PWM_FREQ,
    LED_PWM_PERIOD,
    NULL,
    {LED_PWM_CHANNEL_CFG},
    0,
#if STM32_PWM_USE_ADVANCED
    0,
#endif
    LED_PWM_DMA_TRIGGER,
};
static led_bitfield_t _led_bit_buffer[LED_BIT_BUFFER_SIZE];
static const stm32_dma_stream_t* _led_dma_stream = NULL;
static led_animation_t _led_animation = {
    .type = LED_ANIMATION_NONE,
    .pulse =
        {
            .color = {.h = 0x40, .s = 0x40, .v = 0x40},
            .period = 2000,
        },
};
static uint8_t _led_animation_dirty = 1;

/*
 * Global variables
 */

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_led_animation_thread, arg)
{
  (void)arg;

  //  led_hsv_t rainbow_leds[LED_NUMBER];

  uint32_t cycle = 0;
  uint32_t cycle_period = 0;
  led_rgb_t led;

  chRegSetThreadName("led_animation_th");

  while (true)
  {
    systime_t time = chVTGetSystemTimeX();
    if (_led_animation_dirty)
    {
      switch (_led_animation.type)
      {
        case LED_ANIMATION_STATIC:
          _led_set_led_bitfield_all(&_led_animation.static_color.color);
          break;
        case LED_ANIMATION_PULSE:
          cycle = 0;
          cycle_period = _led_animation.pulse.period / LED_ANIMATION_MAIN_THREAD_P_MS;
          led.r = 0;
          led.g = 0;
          led.b = 0;
          _led_set_led_bitfield_all(&led);
          break;
        case LED_ANIMATION_RAINBOW:
          cycle = 0;
          cycle_period = _led_animation.rainbow.period / LED_ANIMATION_MAIN_THREAD_P_MS;
          break;
        default:
          break;
      }
      chSysLock();
      _led_animation_dirty = 0;
      chSysUnlock();
    }
    else
    {
      cycle = (cycle + 1) % cycle_period;
      switch (_led_animation.type)
      {
        case LED_ANIMATION_PULSE:
        {
          /*
           * Calculate led pulsing based on taylor series representation with n=8:
           * cos x = 1 - x^2/2! + x^4/4! - x^6/6! + x^8/8!
           * We offset the function to 2 and split the calculation at pi/2
           *
           * For x=0...pi:
           *    cos x = 2 - x^2/2! + x^4/4! - x^6/6! + x^8/8!
           * For x=pi...2pi:
           *    cos x = x^2/2! - x^4/4! + x^6/6! - x^8/8!
           */
          const uint16_t fak_2 = 2;
          const uint16_t fak_4 = 24;
          const uint16_t fak_6 = 720;
          const uint16_t fak_8 = 40320;
          const float pi = 3.14159;
          uint32_t half_cycle = cycle_period / 2;
          float x = pi * (float)cycle / (float)(half_cycle);
          x = (x <= pi) ? x : (x - pi);
          float x_2 = x * x;
          float x_4 = x_2 * x_2;
          float x_6 = x_4 * x_2;
          float x_8 = x_6 * x_2;
          float frac_2 = x_2 / (float)fak_2;
          float frac_4 = x_4 / (float)fak_4;
          float frac_6 = x_6 / (float)fak_6;
          float frac_8 = x_8 / (float)fak_8;
          float multiplier = (cycle <= half_cycle) ? (2 - frac_2 + frac_4 - frac_6 + frac_8)
                                                   : (frac_2 - frac_4 + frac_6 - frac_8);
          multiplier = multiplier / 2;
          led_hsv_t pulse_led;
          pulse_led.h = _led_animation.pulse.color.h;
          pulse_led.s = _led_animation.pulse.color.s;
          int16_t v = ((float)_led_animation.pulse.color.v * multiplier);
          pulse_led.v = (v < 0) ? 0 : (v > 255) ? 255 : v;
          _led_hsv_to_rbg(&pulse_led, &led);
          _led_set_led_bitfield_all(&led);
        }
        break;
        case LED_ANIMATION_RAINBOW:
        case LED_ANIMATION_STATIC:
        default:
          break;
      }
    }

    chThdSleepUntilWindowed(time, time + TIME_MS2I(LED_ANIMATION_MAIN_THREAD_P_MS));
  }
}

/*
 * Static helper functions
 */
static void _led_set_led_bitfield(led_rgb_t* src, uint16_t id)
{
  led_bitfield_t* dest = &_led_bit_buffer[id];
  uint8_t i = 0;
  for (i = 0; i < 8; i++)
  {
    dest->r[i] = (src->r & (1 << (8 - i))) ? LED_WS2812B_ONE : LED_WS2812B_ZERO;
    dest->g[i] = (src->g & (1 << (8 - i))) ? LED_WS2812B_ONE : LED_WS2812B_ZERO;
    dest->b[i] = (src->b & (1 << (8 - i))) ? LED_WS2812B_ONE : LED_WS2812B_ZERO;
  }
}

static void _led_set_led_bitfield_all(led_rgb_t* src)
{
  uint8_t i = 0;
  for (i = 0; i < LED_NUMBER; i++)
  {
    _led_set_led_bitfield(src, i);
  }
}

static void _led_hsv_to_rbg(led_hsv_t* hsv, led_rgb_t* rgb)
{
  uint8_t region, remainder, p, q, t;

  if (hsv->s == 0)
  {
    rgb->r = hsv->v;
    rgb->g = hsv->v;
    rgb->b = hsv->v;
    return;
  }

  region = hsv->h / 43;
  remainder = (hsv->h - (region * 43)) * 6;

  p = (hsv->v * (255 - hsv->s)) >> 8;
  q = (hsv->v * (255 - ((hsv->s * remainder) >> 8))) >> 8;
  t = (hsv->v * (255 - ((hsv->s * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
    case 0:
      rgb->r = hsv->v;
      rgb->g = t;
      rgb->b = p;
      break;
    case 1:
      rgb->r = q;
      rgb->g = hsv->v;
      rgb->b = p;
      break;
    case 2:
      rgb->r = p;
      rgb->g = hsv->v;
      rgb->b = t;
      break;
    case 3:
      rgb->r = p;
      rgb->g = q;
      rgb->b = hsv->v;
      break;
    case 4:
      rgb->r = t;
      rgb->g = p;
      rgb->b = hsv->v;
      break;
    default:
      rgb->r = hsv->v;
      rgb->g = p;
      rgb->b = q;
      break;
  }

  return;
}

static void _led_init_hal(void)
{
  pwmStart(LED_PWM_TIMER_DRIVER, &_led_pwm_pwmd_cfg);
  palSetLineMode(LED_PWM_OUTPUT_LINE, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  pwmEnableChannel(LED_PWM_TIMER_DRIVER, (LED_PWM_TIMER_CH - 1), 1);

  // Allocate stream
  _led_dma_stream = dmaStreamAlloc(LED_DMA_STREAM, LED_DMA_IRQ_PRIO, NULL, NULL);

  // Set peripheral address
  dmaStreamSetPeripheral(_led_dma_stream, &(LED_PWM_CCR));

  // Set memory address
  dmaStreamSetMemory0(_led_dma_stream, _led_bit_buffer);

  // set the size of the output buffer
  dmaStreamSetTransactionSize(_led_dma_stream, sizeof(_led_bit_buffer));

  // config BYTE to HWORD transfers, memory to peripheral,
  // inc source address, fixed dest address,
  // circular mode, select channel 4
  dmaStreamSetMode(_led_dma_stream, STM32_DMA_CR_PL(LED_DMA_CH_PRIO) | STM32_DMA_CR_PSIZE_HWORD |
                                        led_dma_transfer_t | STM32_DMA_CR_DIR_M2P |
                                        STM32_DMA_CR_MINC | STM32_DMA_CR_CIRC |
                                        STM32_DMA_CR_CHSEL(LED_DMA_CH));

  dmaStreamEnable(_led_dma_stream);
}

static void _led_init_module(void)
{
  memset(_led_bit_buffer, 0, sizeof(_led_bit_buffer));
  chThdCreateStatic(_led_animation_stack, sizeof(_led_animation_stack), LED_ANIMATION_THREAD_PRIO,
                    _led_animation_thread, NULL);
}

/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void led_loop_ccr_sh(BaseSequentialStream* chp, int argc, char* argv[])
{
  (void)argv;
  if (argc != 1)
  {
    chprintf(chp, "led-loop-ccr period[ms]\r\n");
    return;
  }
  uint32_t period = atoi(argv[0]);
  chprintf(chp, "Dumping capture-compare register:\r\n");
  while (chnGetTimeout((BaseChannel*)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
  {
    systime_t time = chVTGetSystemTimeX();
    chprintf(chp, "0x%08X\r\n", LED_PWM_CCR);
    chThdSleepUntilWindowed(time, time + TIME_MS2I(period));
  }
  chprintf(chp, "\r\n\nstopped\r\n");
}

void led_dump_bit_buffer_sh(BaseSequentialStream* chp, int argc, char* argv[])
{
  (void)argv;
  if (argc > 1)
  {
    chprintf(chp, "led-dump-bit-buffer [raw]\r\n");
    return;
  }
  uint8_t raw_mode = 0;
  uint32_t i = 0;
  uint32_t n = 0;
  uint32_t raw_start = 0;
  uint8_t* raw_buffer = (uint8_t*)&_led_bit_buffer;

  if (argc == 1)
  {
    raw_mode = (strcmp(argv[0], "raw") == 0) ? 1 : 0;
  }
  if (raw_mode == 0)
  {
    chprintf(chp, "Dumping bit buffer:\r\n");
    for (i = 0; i < LED_NUMBER; i++)
    {
      chprintf(chp, "LED %d:\r\n", i);
      chprintf(chp, "  %-3s %-10s %-10s %-10s\r\n", "Bit", "R", "G", "B");
      led_bitfield_t* led = &_led_bit_buffer[i];
      for (n = 0; n < 8; n++)
      {
        uint8_t bit = 7 - n;
        chprintf(chp, "  %-3d 0x%08X 0x%08X 0x%08X\r\n", bit, led->r[n], led->g[n], led->b[n]);
      }
      chprintf(chp, "\r\n");
    }
    chprintf(chp, "Sync gap:\r\n");
    raw_start = sizeof(led_bitfield_t) * LED_NUMBER;
  }
  else
  {
    chprintf(chp, "Dumping raw buffer:\r\n");
  }
  n = 0;
  for (i = raw_start; i < sizeof(_led_bit_buffer); i++)
  {
    chprintf(chp, "0x%08X ", raw_buffer[i]);
    n++;
    if (n % 8 == 0)
    {
      chprintf(chp, "\r\n");
    }
  }
  chprintf(chp, "\r\n");
}
#endif

/*
 * API functions
 */
void led_init(void)
{
  _led_init_hal();
  _led_init_module();
}
