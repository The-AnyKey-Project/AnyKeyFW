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
static void             _glcd_draw_bitmap         (glcd_display_id_t display, glcd_display_buffer_t * object);
static void             _glcd_select_display      (glcd_display_id_t display);
static void             _glcd_unselect_display    (glcd_display_id_t display);
static inline void      _glcd_select_all          (void);
static inline void      _glcd_unselect_all        (void);
static void             _glcd_set_contrast        (uint8_t value);
static void             _glcd_clear_display       (void);
static uint8_t          _glcd_u8g2_hw_spi         (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
static uint8_t          _glcd_u8g2_gpio_and_delay (u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

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
static u8g2_t                   _glcd_display;
static glcd_display_buffer_t *  _glcd_display_buffers = NULL;
static uint8_t                  _glcd_display_buffers_dirty = 0;
static uint32_t                 _glcd_display_cs_lines[GLCD_DISP_MAX] =
{
    GLCD_CS_LINE_1, GLCD_CS_LINE_2, GLCD_CS_LINE_3,
    GLCD_CS_LINE_4, GLCD_CS_LINE_5, GLCD_CS_LINE_6,
    GLCD_CS_LINE_7, GLCD_CS_LINE_8, GLCD_CS_LINE_9
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

  while (true)
  {
    systime_t time = chVTGetSystemTimeX();

    if(_glcd_display_buffers_dirty)
    {
      uint8_t display = 0;
      for (display = 0; display < GLCD_DISP_MAX; display++)
      {
        _glcd_draw_bitmap(display, &_glcd_display_buffers[display]);
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
    palSetLineMode(_glcd_display_cs_lines[display], PAL_MODE_OUTPUT_PUSHPULL);
    palSetLine(_glcd_display_cs_lines[display]);
  }
}

static void _glcd_init_module(void)
{
  _glcd_init_display();
  chThdCreateStatic(_glcd_update_stack, sizeof(_glcd_update_stack), GLCD_UPDATE_THREAD_PRIO, _glcd_update_thread, NULL);
}

static void _glcd_select_display(glcd_display_id_t display)
{
  palClearLine(_glcd_display_cs_lines[display]);
}

static void _glcd_unselect_display(glcd_display_id_t display)
{
  palSetLine(_glcd_display_cs_lines[display]);
}

static inline void _glcd_select_all(void)
{
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
      _glcd_select_display(display);
  }
}

static inline void _glcd_unselect_all(void){
  uint8_t display = 0;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
      _glcd_unselect_display(display);
  }
}

static void _glcd_set_contrast(uint8_t value)
{
  u8g2_SetContrast(&_glcd_display,value);
}

static void _glcd_clear_display(void)
{
  u8g2_ClearDisplay(&_glcd_display);
}

static void _glcd_setup_display(void)
{
  u8g2_Setup_ssd1306_64x48_er_f(&_glcd_display,
                                U8G2_R0,
                                _glcd_u8g2_hw_spi,
                                _glcd_u8g2_gpio_and_delay);

  u8g2_InitDisplay(&_glcd_display);
  _glcd_clear_display();
  u8g2_SetPowerSave(&_glcd_display, 0);
}

static void _glcd_init_display(void)
{

  spiStart(GLCD_SPI_DRIVER, &_glcd_spid_cfg);

  _glcd_select_all();

  /*
   * Since all displays are selected, all displays
   * are initialized at once
   */
  _glcd_setup_display();
  _glcd_set_contrast(GLCD_DEFAULT_BRIGHTNESS);
  _glcd_unselect_all();
}

static void _glcd_draw_bitmap(glcd_display_id_t display, glcd_display_buffer_t * object)
{
  _glcd_select_display(display);
  u8g2_DrawBitmap(&_glcd_display, 0, 0,
                  GLCD_DISPLAY_WIDTH/GLCD_DISPLAY_BLOCK_SIZE,
                  GLCD_DISPLAY_HEIGTH,
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
  switch(msg)
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
  switch(msg)
  {
  //Function to define the logic level of the RESET line
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

  return 1; // command processed successfully.
}

/*
 * Shell functions
 */
extern void glcd_set_contrast(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc != 1) {
    chprintf(chp, "Usage: glcd-set-contrast value (0...255)\r\n");
    return;
  }
  uint32_t value = atoi(argv[0]);
  value = (value > 255 ) ? 255 : value ;

  _glcd_select_all();
  _glcd_set_contrast((uint8_t)(value));
  _glcd_unselect_all();
}

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
