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
 * glcd.c
 *
 *  Created on: 08.01.2021
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
#include "api/hal/glcd.h"

/*
 * Include dependencies
 */
#include "api/hal/flash_storage.h"
#include <stdlib.h>
#include <string.h>
#include "api/app/anykey.h"

/*
 * Forward declarations of static functions
 */
static void _glcd_init_hal(void);
static void _glcd_init_module(void);
static void _glcd_init_display(void);
static void _glcd_draw_bitmap(glcd_display_id_t display, glcd_display_buffer_t *object);
static void _glcd_select_display(glcd_display_id_t display);
static void _glcd_unselect_display(glcd_display_id_t display);
static inline void _glcd_select_all(void);
static inline void _glcd_unselect_all(void);
static void _glcd_set_contrast(uint8_t value);
static void _glcd_clear_display(void);
static uint8_t _glcd_u8g2_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
static uint8_t _glcd_u8g2_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
static void _glcd_reload_contrast(void);

/*
 * Static variables
 */
static const SPIConfig _glcd_spid_cfg = {
    false,
    NULL,          // No finish callback
    GLCD_SPI_CR1,  // SPI_CR1
    GLCD_SPI_CR2   // SPI_CR2
};

static THD_WORKING_AREA(_glcd_update_stack, GLCD_UPDATE_THREAD_STACK);
static u8g2_t _glcd_display;
static uint32_t *_glcd_display_buffers = NULL;
static uint8_t _glcd_display_buffers_dirty = 0;
static mutex_t _glcd_display_mtx[GLCD_DISP_MAX];
static uint8_t _glcd_current_display_contrast[GLCD_DISP_MAX];
static uint32_t _glcd_display_cs_lines[GLCD_DISP_MAX] = {
    GLCD_CS_LINE_1, GLCD_CS_LINE_2, GLCD_CS_LINE_3, GLCD_CS_LINE_4, GLCD_CS_LINE_5,
    GLCD_CS_LINE_6, GLCD_CS_LINE_7, GLCD_CS_LINE_8, GLCD_CS_LINE_9};

/*
 * Global variables
 */

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_glcd_update_thread, arg)
{
  (void)arg;
  systime_t time = 0;
  uint8_t dirty = 0;
  uint8_t display = 0;

  chRegSetThreadName("glcd_update_th");

  /*
   * Poll dirty flag each GLCD_UPDATE_THREAD_P_MS
   */
  while (true)
  {
    time = chVTGetSystemTimeX();

    /*
     * Use critical section to provide
     * consistent data
     */
    chSysLock();
    dirty = _glcd_display_buffers_dirty;
    _glcd_display_buffers_dirty = 0;
    chSysUnlock();

    /*
     * Update displays in case of a set
     * dirty flag
     */
    if (dirty)
    {
      for (display = 0; display < GLCD_DISP_MAX; display++)
      {
        _glcd_draw_bitmap(display,
                          flash_storage_get_pointer_from_idx(_glcd_display_buffers[display]));
      }
    }
    chThdSleepUntilWindowed(time, time + TIME_MS2I(GLCD_UPDATE_THREAD_P_MS));
  }
}

/*
 * Static helper functions
 */
static void _glcd_init_hal(void)
{
  /*
   * Configure SPI driver pins
   *    - SCK
   *    - MOSI
   *    - Data/Command
   *    - Reset
   */
  palSetLineMode(GLCD_SCK_LINE, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetLineMode(GLCD_MOSI_LINE, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
  palSetLineMode(GLCD_DC_LINE, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(GLCD_RESET_LINE, PAL_MODE_OUTPUT_PUSHPULL);

  /*
   * Configure cs pins
   */
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    palSetLineMode(_glcd_display_cs_lines[display], PAL_MODE_OUTPUT_PUSHPULL);
    palSetLine(_glcd_display_cs_lines[display]);
  }
}

static void _glcd_init_module(void)
{
  uint8_t display = 0;
  /*
   * Create mutex lock for each display
   */
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    chMtxObjectInit(&_glcd_display_mtx[display]);
  }

  /*
   * Initialize displays
   */
  _glcd_init_display();

  /*
   * Create glcd update task
   */
  chThdCreateStatic(_glcd_update_stack, sizeof(_glcd_update_stack), GLCD_UPDATE_THREAD_PRIO,
                    _glcd_update_thread, NULL);
}

static void _glcd_select_display(glcd_display_id_t display)
{
  /*
   * Set CS line for mutex lock protected display
   */
  chMtxLock(&_glcd_display_mtx[display]);
  palClearLine(_glcd_display_cs_lines[display]);
}

static void _glcd_unselect_display(glcd_display_id_t display)
{
  /*
   * Reset CS line and release mutex lock for display
   */
  palSetLine(_glcd_display_cs_lines[display]);
  chMtxUnlock(&_glcd_display_mtx[display]);
}

static inline void _glcd_select_all(void)
{
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    _glcd_select_display(display);
  }
}

static inline void _glcd_unselect_all(void)
{
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    _glcd_unselect_display(display);
  }
}

static void _glcd_set_contrast(uint8_t value)
{
  /*
   * Simple forward to u8g2 function, stuck
   * to the common u8g2 handle
   */
  u8g2_SetContrast(&_glcd_display, value);
}

static void _glcd_clear_display(void)
{
  /*
   * Simple forward to u8g2 function, stuck
   * to the common u8g2 handle
   */
  u8g2_ClearDisplay(&_glcd_display);
}

static void _glcd_setup_display(void)
{
  /*
   * Setup full buffer u8g2 driver for
   * 64x48 pixel ssd1306 based display
   */
  u8g2_Setup_ssd1306_64x48_er_f(&_glcd_display, U8G2_R0, _glcd_u8g2_hw_spi,
                                _glcd_u8g2_gpio_and_delay);

  /*
   * Initialize u8g2 handle, clear all
   * displays (called from _glcd_init_display)
   * and set power mode
   */
  u8g2_InitDisplay(&_glcd_display);
  _glcd_clear_display();
  u8g2_SetPowerSave(&_glcd_display, 0);
}

static void _glcd_init_display(void)
{
  /*
   * Start SPI driver and select all displays
   */
  spiStart(GLCD_SPI_DRIVER, &_glcd_spid_cfg);
  _glcd_select_all();

  /*
   * Since all displays are selected, all displays
   * are initialized at once
   */
  _glcd_setup_display();
  _glcd_unselect_all();
  _glcd_reload_contrast();
}

static void _glcd_draw_bitmap(glcd_display_id_t display, glcd_display_buffer_t *object)
{
  /*
   * Draw bitmap for selected display
   */
  _glcd_select_display(display);
  u8g2_DrawBitmap(&_glcd_display, object->header.x_offset, object->header.y_offset,
                  object->header.x_size / GLCD_DISPLAY_BLOCK_SIZE, object->header.y_size,
                  object->content);
  u8g2_SendBuffer(&_glcd_display);
  _glcd_unselect_display(display);
}

/*
 * Callback functions
 */
static uint8_t _glcd_u8g2_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  (void)u8x8;
  /*
   * u8g2 callback function for SPI interaction
   * We only need to consider
   *    - Send byte
   *    - Start transfer (no action, only return 1)
   *    - End transfer (no action, only return 1)
   *    - Display Data/Command selection
   */
  switch (msg)
  {
    case U8X8_MSG_BYTE_SEND:
      spiSend(GLCD_SPI_DRIVER, arg_int, arg_ptr);
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      break;
    case U8X8_MSG_BYTE_SET_DC:
      palWriteLine(GLCD_DC_LINE, arg_int);
      break;
    default:
      return 0;
  }
  return 1;
}

static uint8_t _glcd_u8g2_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  (void)u8x8;
  (void)arg_ptr;
  /*
   * u8g2 callback function for gpio and delay
   * We only need to consider
   *    - Millisecond delay
   *    - Display Reset
   *    - Display Data/Command selection
   */
  switch (msg)
  {
    case U8X8_MSG_DELAY_MILLI:
      chThdSleepMilliseconds(arg_int);
      break;
    case U8X8_MSG_GPIO_RESET:
      palWriteLine(GLCD_RESET_LINE, arg_int);
      break;
    case U8X8_MSG_GPIO_DC:
      palWriteLine(GLCD_DC_LINE, arg_int);
      break;
    default:
      return 1;
  }

  return 1;  // command processed successfully.
}

static void _glcd_reload_contrast(void)
{
  uint8_t display = 0;

  /*
   * Reload contrast values from flash
   * and set them for each display
   */
  flash_storage_get_display_contrast(_glcd_current_display_contrast);

  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    glcd_set_contrast(display, _glcd_current_display_contrast[display]);
  }
}

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void glcd_set_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 2)
  {
    chprintf(chp, "Usage: glcd-set-contrast (display) (contrast)\r\n");
    chprintf(chp, "  Display range:  %d...%d\r\n", GLCD_DISP_1, (GLCD_DISP_MAX - 1));
    chprintf(chp, "  Contrast range: 0...255\r\n");
    return;
  }
  uint8_t display = atoi(argv[0]);
  uint32_t value = atoi(argv[1]);
  value = (value > 255) ? 255 : value;

  if (glcd_set_contrast(display, value) == 0)
  {
    chprintf(chp, "Display id out of range (%d...%d)\r\n", GLCD_DISP_1, GLCD_DISP_MAX);
  }
}

void glcd_get_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage: glcd-get-contrast\r\n");
    return;
  }

  chprintf(chp, "Display    0   1   2   3   4   5   6   7   8\r\n");
  chprintf(chp, "Contrast ");
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    chprintf(chp, "%3d ", _glcd_current_display_contrast[display]);
  }
  chprintf(chp, "\r\n");
}

void glcd_reload_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 0)
  {
    chprintf(chp, "Usage: glcd-reload-contrast\r\n");
    return;
  }

  chprintf(chp, "Reloading contrast values form flash...");
  _glcd_reload_contrast();
  chprintf(chp, "done!\r\n");
}
#endif

/*
 * API functions
 */
void glcd_init(void)
{
  _glcd_init_hal();
  _glcd_init_module();
}

void glcd_set_displays(uint32_t *buffer)
{
  /*
   * Use critical section to provide
   * consistent data
   */
  chSysLockFromISR();
  _glcd_display_buffers = buffer;
  _glcd_display_buffers_dirty = 1;
  chSysUnlockFromISR();
}

uint8_t glcd_set_contrast(glcd_display_id_t display, uint8_t value)
{
  uint8_t ret = 1;
  if (display < GLCD_DISP_MAX)
  {
    /*
     * Use critical section to provide
     * consistent data
     */
    chSysLock();
    _glcd_current_display_contrast[display] = value;
    chSysUnlock();

    /*
     * Set contrast for selected display
     */
    _glcd_select_display(display);
    _glcd_set_contrast((uint8_t)(value));
    _glcd_unselect_display(display);
  }
  else
  {
    ret = 0;
  }
  return ret;
}

uint8_t glcd_get_contrast(glcd_display_id_t display)
{
  uint8_t ret = 0;
  if (display < GLCD_DISP_MAX)
  {
    /*
     * Use critical section to provide
     * consistent data
     */
    chSysLock();
    ret = _glcd_current_display_contrast[display];
    chSysUnlock();
  }
  return ret;
}
