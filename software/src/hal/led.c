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
static void _led_get_animation(led_animation_t* dest);
static void _led_set_led_bitfield(led_rgb_t* src, uint16_t id);
static void _led_set_led_bitfield_all(led_rgb_t* src);
static void _led_clear_all(void);
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
static led_animation_t _led_animation = {.type = LED_ANIMATION_NONE};
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
  systime_t time = 0;
  uint32_t i = 0;
  uint32_t cycle = 0;
  uint32_t cycle_period = 0;
  led_rgb_t led_rgb;
  led_hsv_t led_hsv;
  led_animation_t animation;
  uint8_t dirty = 0;
  float rainbow_leds_h[LED_NUMBER];

  chRegSetThreadName("led_animation_th");

  while (true)
  {
    time = chVTGetSystemTimeX();
    _led_get_animation(&animation);

    /*
     * Use critical section to provide
     * consistent data
     */
    chSysLock();
    dirty = _led_animation_dirty;
    _led_animation_dirty = 0;
    chSysUnlock();

    if (dirty)
    {
      switch (animation.type)
      {
        case LED_ANIMATION_NONE:
          _led_clear_all();
          break;
        case LED_ANIMATION_STATIC:
          /*
           * Set color once, no animation settings needed
           */
          _led_set_led_bitfield_all(&animation.static_color.color);
          break;
        case LED_ANIMATION_PULSE:
          /*
           * Set cycle period to define pulsing speed
           */
          cycle = 0;
          cycle_period = animation.pulse.period / LED_ANIMATION_MAIN_THREAD_P_MS;
          _led_clear_all();
          break;
        case LED_ANIMATION_RAINBOW:
          /*
           * Calculate H offset values for rainbow pattern
           */
          cycle = 0;
          cycle_period = animation.rainbow.period / LED_ANIMATION_MAIN_THREAD_P_MS;
          for (i = 0; i < LED_NUMBER; i++)
          {
            rainbow_leds_h[i] = (float)(255 * i) / (float)(animation.rainbow.leds_per_rainbow);
          }
          _led_clear_all();
          break;
        default:
          break;
      }
    }
    else
    {
      cycle = (cycle + 1) % cycle_period;
      switch (animation.type)
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
          led_hsv.h = animation.pulse.color.h;
          led_hsv.s = animation.pulse.color.s;
          int16_t v = ((float)animation.pulse.color.v * multiplier);
          led_hsv.v = (v < 0) ? 0 : (v > 255) ? 255
                                              : v;
          _led_hsv_to_rbg(&led_hsv, &led_rgb);
          _led_set_led_bitfield_all(&led_rgb);
        }
        break;
        case LED_ANIMATION_RAINBOW:
        {
          /*
           * Move the H value (rotate) of each LED by a step size, based
           * on the given cycle and cycle period
           */
          float rainbow_inc = (float)255 * (float)cycle / (float)cycle_period;
          for (i = 0; i < LED_NUMBER; i++)
          {
            uint16_t h = (uint16_t)(rainbow_leds_h[i] + rainbow_inc);
            led_hsv.h = h % 256;
            led_hsv.s = animation.rainbow.s;
            led_hsv.v = animation.rainbow.v;
            _led_hsv_to_rbg(&led_hsv, &led_rgb);
            _led_set_led_bitfield(&led_rgb, i);
          }
          break;
        }
        case LED_ANIMATION_NONE:
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
static void _led_get_animation(led_animation_t* dest)
{
  /*
   * Use critical section to provide
   * consistent data
   */
  chSysLock();
  memcpy(dest, &_led_animation, sizeof(led_animation_t));
  chSysUnlock();
}

static void _led_set_led_bitfield(led_rgb_t* src, uint16_t id)
{
  led_bitfield_t* dest = &_led_bit_buffer[id];
  uint8_t i = 0;
  /*
   * Loop bitwise and set the corresponding CCR value for
   * 0 (0.35 µs) or 1 (0.9µs) to the bit buffer array.
   */
  for (i = 0; i < 8; i++)
  {
    dest->r[i] = (src->r & (1 << (8 - i))) ? LED_WS2812B_ONE : LED_WS2812B_ZERO;
    dest->g[i] = (src->g & (1 << (8 - i))) ? LED_WS2812B_ONE : LED_WS2812B_ZERO;
    dest->b[i] = (src->b & (1 << (8 - i))) ? LED_WS2812B_ONE : LED_WS2812B_ZERO;
  }
}

static void _led_set_led_bitfield_all(led_rgb_t* src)
{
  /*
   * Loop over all LEDs an set the corresponding values
   */
  uint8_t i = 0;
  for (i = 0; i < LED_NUMBER; i++)
  {
    _led_set_led_bitfield(src, i);
  }
}

static void _led_clear_all(void)
{
  led_rgb_t led;
  led.r = 0;
  led.g = 0;
  led.b = 0;
  _led_set_led_bitfield_all(&led);
}

static void _led_hsv_to_rbg(led_hsv_t* hsv, led_rgb_t* rgb)
{
  /*
   * Magic conversion from HSV -> RGB. Found a similar
   * version somewhere in the internet, don't ask me
   * what the code is doing
   */
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
  /*
   * Initialize PWM driver, enable output and PWM driver
   */
  pwmStart(LED_PWM_TIMER_DRIVER, &_led_pwm_pwmd_cfg);
  palSetLineMode(LED_PWM_OUTPUT_LINE, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  pwmEnableChannel(LED_PWM_TIMER_DRIVER, (LED_PWM_TIMER_CH - 1), 1);

  /*
   * Initialize DMA stream, for automated transfer between
   * bit buffer and capture compare register (CCR). According to the
   * default timer configuration, a DMA request is generated when
   * CCR == CNT. The DMA transfer writes to a shadowed CCR, which
   * becomes active on timer overrun, setting the new compare value
   * for the next PWM period.
   *
   *   - Allocate DMA stream
   *   - Set CCR as peripheral address
   *   - Set memory base address to _led_bit_buffer
   *   - Set transaction size to sizeof(_led_bit_buffer)
   *   - Configure DMA transfer
   *            - Channel, including priority
   *            - Direction memory -> periphery
   *            - Source size according to led_dma_transfer_t
   *            - Target size is half word according to CCR definition
   *            - Increment memory position after each transfer
   *            - Circular mode
   *   - Enable stream
   */
  _led_dma_stream = dmaStreamAlloc(LED_DMA_STREAM, LED_DMA_IRQ_PRIO, NULL, NULL);
  dmaStreamSetPeripheral(_led_dma_stream, &(LED_PWM_CCR));
  dmaStreamSetMemory0(_led_dma_stream, _led_bit_buffer);
  dmaStreamSetTransactionSize(_led_dma_stream, sizeof(_led_bit_buffer));
  dmaStreamSetMode(_led_dma_stream, STM32_DMA_CR_PL(LED_DMA_CH_PRIO) | STM32_DMA_CR_PSIZE_HWORD |
                                        led_dma_transfer_t | STM32_DMA_CR_DIR_M2P |
                                        STM32_DMA_CR_MINC | STM32_DMA_CR_CIRC |
                                        STM32_DMA_CR_CHSEL(LED_DMA_CH));
  dmaStreamEnable(_led_dma_stream);
}

static void _led_init_module(void)
{
  /*
   * Initialize bit buffer for PWM signal
   */
  memset(_led_bit_buffer, 0, sizeof(_led_bit_buffer));

  /*
   * Create led task for animation processing
   */
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

void led_set_animation(led_animation_t* animation)
{
  /*
   * Use critical section to provide
   * consistent data
   */
  chSysLock();
  memcpy(&_led_animation, animation, sizeof(led_animation_t));
  _led_animation_dirty = 1;
  chSysUnlock();
}
