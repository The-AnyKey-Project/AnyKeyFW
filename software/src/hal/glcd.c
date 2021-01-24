/*
 * glcd.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

/*
 * Include ChibiOS & HAL
 */
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

/*
 * Includes module API, types & config
 */
#include "api/hal/glcd.h"

/*
 * Include dependencies
 */
#include <stdlib.h>
#include <string.h>
#include "api/app/anykey.h"

/*
 * Forward declarations of static functions
 */
static void             _glcd_init_hal            (void);
static void             _glcd_init_module         (void);
static void             _glcd_init_display        (void);
static void             _glcd_draw_bitmap         (u8g2_t *u8g2, glcd_display_buffer_t * object);
static void             _glcd_select_display      (glcd_display_id_t display);
static void             _glcd_unselect_display    (glcd_display_id_t display);
static uint8_t          _glcd_u8g2_hw_spi         (glcd_display_id_t disp, U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr);
static uint8_t          _glcd_u8g2_gpio_and_delay (U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr);
static uint8_t          _glcd_spi_cb_1            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_1, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_2            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_2, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_3            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_3, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_4            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_4, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_5            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_5, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_6            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_6, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_7            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_7, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_8            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_8, u8x8, msg, arg_int, arg_ptr); }
static uint8_t          _glcd_spi_cb_9            (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) { return _glcd_u8g2_hw_spi(GLCD_DISP_9, u8x8, msg, arg_int, arg_ptr); }

/*
 * Static variables
 */
static const SPIConfig _glcd_spid_cfg = {
  false,
  NULL,              // No finish callback
  GLCD_SPI_CR1,      // SPI_CR1
  GLCD_SPI_CR2       // SPI_CR2
};

static THD_WORKING_AREA(_glcd_update_stack, GLCD_UPDATE_THREAD_STACK);
static uint8_t                  _glcd_msg_buffer[GLCD_SPI_BUFFER_SIZE];
static uint16_t                 _glcd_msg_buffer_idx = 0;
static glcd_display_buffer_t *  _glcd_display_buffers = NULL;
static uint8_t                  _glcd_display_buffers_dirty = 0;
static glcd_display_t           _glcd_displays[GLCD_DISP_MAX] =
{
    {.cs_line = GLCD_CS_LINE_1, .spi_cb = &_glcd_spi_cb_1, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_2, .spi_cb = &_glcd_spi_cb_2, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_3, .spi_cb = &_glcd_spi_cb_3, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_4, .spi_cb = &_glcd_spi_cb_4, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_5, .spi_cb = &_glcd_spi_cb_5, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_6, .spi_cb = &_glcd_spi_cb_6, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_7, .spi_cb = &_glcd_spi_cb_7, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_8, .spi_cb = &_glcd_spi_cb_8, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
    {.cs_line = GLCD_CS_LINE_9, .spi_cb = &_glcd_spi_cb_9, .gpio_cb = &_glcd_u8g2_gpio_and_delay },
};


/*
 * Global variables
 */

/*
 * Tasks
 */
static __attribute__((noreturn)) THD_FUNCTION(_glcd_update_thread, arg)
{

  (void)arg;

  chRegSetThreadName("glcd_update");
  chThdSleepMilliseconds(GLCD_UPDATE_THREAD_P_MS);

  while (true)
  {
    systime_t time = chVTGetSystemTimeX();

    if(_glcd_display_buffers_dirty)
    {
      uint8_t display = 0;
      for (display = 0; display < GLCD_DISP_MAX; display++)
      {
        _glcd_draw_bitmap(&_glcd_displays[display].handle, &_glcd_display_buffers[display]);
      }
      chSysLock();
      _glcd_display_buffers_dirty = 0;
      chSysUnlock();
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
   */
  palSetLineMode(GLCD_SCK_LINE,   PAL_MODE_STM32_ALTERNATE_PUSHPULL);     // SCK
  palSetLineMode(GLCD_MOSI_LINE,  PAL_MODE_STM32_ALTERNATE_PUSHPULL);     // MOSI
  palSetLineMode(GLCD_DC_LINE,    PAL_MODE_OUTPUT_PUSHPULL);              // Data/Command
  palSetLineMode(GLCD_RESET_LINE, PAL_MODE_OUTPUT_PUSHPULL);              // Reset

  /*
   * Configure cs pins
   */
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    palSetLineMode(_glcd_displays[display].cs_line, PAL_MODE_OUTPUT_PUSHPULL);
  }
}

static void _glcd_init_module(void)
{
  _glcd_init_display();
  chThdCreateStatic(_glcd_update_stack, sizeof(_glcd_update_stack), GLCD_UPDATE_THREAD_PRIO, _glcd_update_thread, NULL);
}

static void _glcd_select_display(glcd_display_id_t display)
{
  palClearLine(_glcd_displays[display].cs_line);
}

static void _glcd_unselect_display(glcd_display_id_t display)
{
  palSetLine(_glcd_displays[display].cs_line);
}

static void _glcd_init_display(void)
{
  uint8_t display = 0;

  spiStart(GLCD_SPI_DRIVER, &_glcd_spid_cfg);

  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
      _glcd_unselect_display((glcd_display_id_t)display);
    u8g2_Setup_ssd1306_64x48_er_f(&_glcd_displays[display].handle,
                                  U8G2_R0,
                                  _glcd_displays[display].spi_cb,
                                  _glcd_displays[display].gpio_cb);
    u8g2_InitDisplay(&_glcd_displays[display].handle);
    u8g2_SetPowerSave(&_glcd_displays[display].handle, 0);
    u8g2_SetContrast(&_glcd_displays[display].handle,0);
  }
}

static void _glcd_draw_bitmap(u8g2_t *u8g2, glcd_display_buffer_t * object)
{
  u8g2_DrawBitmap(u8g2, 0, 0,
                  GLCD_DISPLAY_WIDTH/GLCD_DISPLAY_BLOCK_SIZE,
                  GLCD_DISPLAY_HEIGTH/GLCD_DISPLAY_BLOCK_SIZE,
                  object->content);
  u8g2_SendBuffer(u8g2);
}

/*
 * Callback functions
 */
static uint8_t _glcd_u8g2_hw_spi(glcd_display_id_t disp, U8X8_UNUSED u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{

  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      memcpy(&_glcd_msg_buffer[_glcd_msg_buffer_idx], arg_ptr, arg_int);
      _glcd_msg_buffer_idx += arg_int;
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      _glcd_msg_buffer_idx = 0;
      _glcd_select_display(disp);
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      spiSend(GLCD_SPI_DRIVER, _glcd_msg_buffer_idx, _glcd_msg_buffer);
      _glcd_unselect_display(disp);
      break;
    case U8X8_MSG_BYTE_SET_DC:
      if (arg_int) palSetLine(GLCD_DC_LINE);
      else palClearLine(GLCD_DC_LINE);
      break;
    default:
      return 0;
  }
  return 1;
}

static uint8_t _glcd_u8g2_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
  switch(msg)
  {
  //Function to define the logic level of the RESET line
    case U8X8_MSG_DELAY_NANO:
      {
        uint16_t i = 0;
        for(i=0; i < arg_int; i++)
                asm volatile("nop");
      }
      break;
    case U8X8_MSG_DELAY_MILLI:
      {
        uint32_t i = 0;
        for(i=0; i < arg_int*1000; i++)
                asm volatile("nop");
      }
      break;
    case U8X8_MSG_GPIO_RESET:
      if (arg_int) palSetLine(GLCD_RESET_LINE);
      else palClearLine(GLCD_RESET_LINE);
      break;
    default:
      return 1;
  }

  return 1; // command processed successfully.
}

/*
 * Shell functions
 */

/*
 * API functions
 */
void glcd_init(void)
{
  _glcd_init_hal();
  _glcd_init_module();
}

void glcd_set_displays(glcd_display_buffer_t * buffer)
{
  chSysLockFromISR();
  _glcd_display_buffers = buffer;
  _glcd_display_buffers_dirty = 1;
  chSysUnlockFromISR();
}
