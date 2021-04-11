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
 * flash_storage.c
 *
 *  Created on: 04.03.2021
 *      Author: matthiasb85
 */

/*
 * Include ChibiOS & HAL
 */
// clang-format off
#include "ch.h"
#include "hal.h"
#include "hal_community.h"
#include "chprintf.h"
// clang-format on

/*
 * Includes module API, types & config
 */
#include "api/hal/flash_storage.h"
#include "api/hal/usb.h"

/*
 * Include dependencies
 */
#include "api/app/anykey.h"
#include <string.h>
#include <stddef.h>

/*
 * Forward declarations of static functions
 */
static void _flash_storage_init_hal(void);
static void _flash_storage_init_module(void);
static void _flash_storage_write_default_config(void);
static uint32_t _flash_storage_get_crc(void);
static uint16_t _flash_storage_get_first_sector(void);
#if defined(USE_CMD_SHELL)
static uint8_t _flash_storage_verify_config(uint8_t *config, uint32_t size);
#endif

/*
 * Static variables
 */
static uint8_t *_flash_storage_area = NULL;
static const flash_storage_default_layer_t _flash_storage_default_layer = {
    .flash_header =
        {
            .crc = 0x93606858,
            .version = FLASH_STORAGE_HEADER_VERSION,
            .initial_layer_idx = offsetof(flash_storage_default_layer_t, l1_header),
            .first_layer_idx = offsetof(flash_storage_default_layer_t, l1_header),
            .display_contrast = {GLCD_DEFAULT_BRIGHTNESS, GLCD_DEFAULT_BRIGHTNESS,
                                 GLCD_DEFAULT_BRIGHTNESS, GLCD_DEFAULT_BRIGHTNESS,
                                 GLCD_DEFAULT_BRIGHTNESS, GLCD_DEFAULT_BRIGHTNESS,
                                 GLCD_DEFAULT_BRIGHTNESS, GLCD_DEFAULT_BRIGHTNESS,
                                 GLCD_DEFAULT_BRIGHTNESS},
        },
    .l1_header =
        {
            .next_idx = offsetof(flash_storage_default_layer_t, l2_header),
            .prev_idx = 0,
            .name_idx = offsetof(flash_storage_default_layer_t, l1_name),
            .display_idx =
                {
                    offsetof(flash_storage_default_layer_t, db[0]),
                    offsetof(flash_storage_default_layer_t, db[1]),
                    offsetof(flash_storage_default_layer_t, db[2]),
                    offsetof(flash_storage_default_layer_t, db[3]),
                    offsetof(flash_storage_default_layer_t, db[4]),
                    offsetof(flash_storage_default_layer_t, db[5]),
                    offsetof(flash_storage_default_layer_t, db[6]),
                    offsetof(flash_storage_default_layer_t, db[7]),
                    offsetof(flash_storage_default_layer_t, db[8]),
                },
            .key_action_press_idx =
                {
                    offsetof(flash_storage_default_layer_t, kp_0),
                    offsetof(flash_storage_default_layer_t, kp_1),
                    offsetof(flash_storage_default_layer_t, kp_2),
                    offsetof(flash_storage_default_layer_t, kp_3),
                    offsetof(flash_storage_default_layer_t, kp_4),
                    offsetof(flash_storage_default_layer_t, kp_5),
                    offsetof(flash_storage_default_layer_t, kp_6),
                    offsetof(flash_storage_default_layer_t, kp_7),
                    offsetof(flash_storage_default_layer_t, kp_8),
                },
            .key_action_release_idx =
                {
                    offsetof(flash_storage_default_layer_t, kr_0),
                    offsetof(flash_storage_default_layer_t, kr_1),
                    offsetof(flash_storage_default_layer_t, kr_2),
                    offsetof(flash_storage_default_layer_t, kr_3),
                    offsetof(flash_storage_default_layer_t, kr_4),
                    offsetof(flash_storage_default_layer_t, kr_5),
                    offsetof(flash_storage_default_layer_t, kr_6),
                    0,
                    0,
                },
            .led_animation =
                {
                    .type = LED_ANIMATION_PULSE,
                    .pulse.color =
                        {
                            .h = 0x80,
                            .s = 0xff,
                            .v = 0x80,
                        },
                    .pulse.period = 4000,
                },
        },
    .l2_header =
        {
            .next_idx = 0,
            .prev_idx = offsetof(flash_storage_default_layer_t, l1_header),
            .name_idx = offsetof(flash_storage_default_layer_t, l2_name),
            .display_idx =
                {
                    offsetof(flash_storage_default_layer_t, db[8]),
                    offsetof(flash_storage_default_layer_t, db[7]),
                    offsetof(flash_storage_default_layer_t, db[6]),
                    offsetof(flash_storage_default_layer_t, db[5]),
                    offsetof(flash_storage_default_layer_t, db[4]),
                    offsetof(flash_storage_default_layer_t, db[3]),
                    offsetof(flash_storage_default_layer_t, db[2]),
                    offsetof(flash_storage_default_layer_t, db[1]),
                    offsetof(flash_storage_default_layer_t, db[0]),
                },
            .key_action_press_idx =
                {
                    offsetof(flash_storage_default_layer_t, kp_8),
                    offsetof(flash_storage_default_layer_t, kp_7),
                    offsetof(flash_storage_default_layer_t, kp_6),
                    offsetof(flash_storage_default_layer_t, kp_5),
                    offsetof(flash_storage_default_layer_t, kp_4),
                    offsetof(flash_storage_default_layer_t, kp_3),
                    offsetof(flash_storage_default_layer_t, kp_2),
                    offsetof(flash_storage_default_layer_t, kp_1),
                    offsetof(flash_storage_default_layer_t, kp_0),
                },
            .key_action_release_idx =
                {
                    0,
                    0,
                    offsetof(flash_storage_default_layer_t, kr_6),
                    offsetof(flash_storage_default_layer_t, kr_5),
                    offsetof(flash_storage_default_layer_t, kr_4),
                    offsetof(flash_storage_default_layer_t, kr_3),
                    offsetof(flash_storage_default_layer_t, kr_2),
                    offsetof(flash_storage_default_layer_t, kr_1),
                    offsetof(flash_storage_default_layer_t, kr_0),
                },
            .led_animation =
                {
                    .type = LED_ANIMATION_RAINBOW,
                    .rainbow.leds_per_rainbow = LED_NUMBER,
                    .rainbow.period = 4000,
                    .rainbow.s = 0xff,
                    .rainbow.v = 0x80,
                },
        },
    .l1_name = FLASH_STORAGE_DEFCONFIG_L1_NAME,
    .l2_name = FLASH_STORAGE_DEFCONFIG_L2_NAME,
    .db =
        {
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_COPY)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_CUT)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_PASTE)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_VOL_MUTE)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_VOL_DEC)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_VOL_INC)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_WWW)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_CONTRAST_DEC)},
            {FLASH_STORAGE_DB_CONTENT(FLASH_STORAGE_DEFCONFIG_IMAGE_CONTRAST_INC)},
        },
    .kp_0 =
        {
            .length = sizeof(anykey_action_key_t),
            .act_1 = FLASH_STORAGE_KEY_PRESS(0x01, 0x06),  // ctrl + c
        },
    .kp_1 =
        {
            .length = sizeof(anykey_action_key_t),
            .act_1 = FLASH_STORAGE_KEY_PRESS(0x01, 0x1B),  // ctrl + x
        },
    .kp_2 =
        {
            .length = sizeof(anykey_action_key_t),
            .act_1 = FLASH_STORAGE_KEY_PRESS(0x01, 0x19),  // ctrl + v
        },
    .kp_3 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_PRESS(USB_HID_REPORT_ID_CONSUMER, 0xE2),  // vol mute
        },
    .kp_4 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_PRESS(USB_HID_REPORT_ID_CONSUMER, 0xEA),  // vol -
        },
    .kp_5 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_PRESS(USB_HID_REPORT_ID_CONSUMER, 0xE9),  // vol +
        },
    .kp_6 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_PRESS(USB_HID_REPORT_ID_CONSUMER, 0x0196),  // browser
        },
    .kp_7 =
        {
            .length = sizeof(anykey_action_contrast_t),
            .act_1 = FLASH_STORAGE_CONTRAST_PRESS(-10),  // dec contrast
        },
    .kp_8 =
        {
            .length = sizeof(anykey_action_contrast_t),
            .act_1 = FLASH_STORAGE_CONTRAST_PRESS(10),  // inc contrast
        },
    .kr_0 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = FLASH_STORAGE_KEY_RELEASE(0x01, 0x06),  // release c
            .act_2 = FLASH_STORAGE_KEY_PRESS(0x00, 0x00),    // release all
        },
    .kr_1 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = FLASH_STORAGE_KEY_RELEASE(0x01, 0x1B),  // release x
            .act_2 = FLASH_STORAGE_KEY_PRESS(0x00, 0x00),    // release all
        },
    .kr_2 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = FLASH_STORAGE_KEY_RELEASE(0x01, 0x19),  // release v
            .act_2 = FLASH_STORAGE_KEY_PRESS(0x00, 0x00),    // release all
        },
    .kr_3 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_RELEASE(USB_HID_REPORT_ID_CONSUMER,
                                                  0xE2),  // release vol 0
        },
    .kr_4 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_RELEASE(USB_HID_REPORT_ID_CONSUMER,
                                                  0xEA),  // release vol -
        },
    .kr_5 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_RELEASE(USB_HID_REPORT_ID_CONSUMER,
                                                  0xE9),  // release vol +
        },
    .kr_6 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = FLASH_STORAGE_KEYEXT_RELEASE(USB_HID_REPORT_ID_CONSUMER,
                                                  0x0196),  // release browser
        },
};
/*
 * Global variables
 */

extern uint32_t __flash1_base__;

/*
 * Tasks
 */

/*
 * Static helper functions
 */
static void _flash_storage_init_hal(void)
{
  /*
   * Initialize flash driver
   */
  eflStart(&FLASH_STORAGE_DRIVER_HANDLE, NULL);

  /*
   * Enable and initialize CRC driver.
   * The hardware module uses CRC-32 (Ethernet)
   * polynomial: 0x4C11DB7
   */
  rccEnableCRC(true);
  crcStart(&FLASH_STORAGE_CRC_HANDLE, NULL);
}

static void _flash_storage_init_module(void)
{
  /*
   * Initialize _flash_storage_area to flash1 section
   */
  _flash_storage_area = (uint8_t *)&__flash1_base__;

  /*
   * Calculate and check CRC, overwrite flash with default
   * configuration if header CRC does not match.
   */
  uint32_t crc = _flash_storage_get_crc();
  flash_storage_header_t *header = (flash_storage_header_t *)_flash_storage_area;
  if (crc != header->crc)
  {
    _flash_storage_write_default_config();
  }
}

static void _flash_storage_write_default_config(void)
{
  uint16_t start_sector = _flash_storage_get_first_sector();
  uint16_t i = 0;

  const flash_descriptor_t *desc = efl_lld_get_descriptor(&FLASH_STORAGE_DRIVER_HANDLE);
  flash_offset_t flash_offset = (flash_offset_t)_flash_storage_area - (flash_offset_t)desc->address;

  /*
   * Erase all flash sectors in flash1 section
   */
  for (i = start_sector; i <= FLASH_STORAGE_LAST_SECTOR; i++)
  {
    uint32_t wait_time = 0;
    efl_lld_start_erase_sector(&FLASH_STORAGE_DRIVER_HANDLE, (flash_sector_t)i);
    efl_lld_query_erase(&FLASH_STORAGE_DRIVER_HANDLE, &wait_time);
    chThdSleep(TIME_MS2I(wait_time));
  }

  /*
   * Write default config
   */
  efl_lld_program(&FLASH_STORAGE_DRIVER_HANDLE, flash_offset, sizeof(flash_storage_default_layer_t),
                  (const uint8_t *)&_flash_storage_default_layer);
}

static uint32_t _flash_storage_get_crc(void)
{
  /*
   * Use hardware CRC module to calculate flash CRC
   */
  crcResetI(&FLASH_STORAGE_CRC_HANDLE);
  return crcCalcI(&FLASH_STORAGE_CRC_HANDLE, FLASH_STORAGE_SIZE - sizeof(crc_t),
                  &_flash_storage_area[sizeof(crc_t)]);
}

static uint16_t _flash_storage_get_first_sector(void)
{
  /*
   * Calculate absolute index of first flash sector
   * used for configuration storing
   */
  const flash_descriptor_t *desc = efl_lld_get_descriptor(&FLASH_STORAGE_DRIVER_HANDLE);
  return FLASH_STORAGE_LAST_SECTOR - FLASH_STORAGE_SIZE / desc->sectors_size;
}

#if defined(USE_CMD_SHELL)
static uint8_t _flash_storage_verify_config(uint8_t *config, uint32_t size)
{
  uint32_t i = 0;
  for (i = 0; i < size; i++)
  {
    if (config[i] != _flash_storage_area[i]) return 0;
  }

  return 1;
}
#endif

/*
 * Callback functions
 */

#if defined(USE_CMD_SHELL)
/*
 * Shell functions
 */
void flash_storage_info_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;

  if (argc != 0)
  {
    chprintf(chp, "Usage: fs-info name\r\n");
    return;
  }
  flash_storage_header_t *header = (flash_storage_header_t *)_flash_storage_area;
  uint32_t crc = _flash_storage_get_crc();
  if (header->crc != crc)
  {
    chprintf(chp, "Warning CRC missmatch!\r\nActual CRC of flash partition is 0x%08x\r\n", crc);
  }
  chprintf(chp, "Flash partition starts at 0x%08p with size of %d bytes\r\n\r\n", header,
           FLASH_STORAGE_SIZE);

  chprintf(chp, "CRC           0x%08x\r\n", header->crc);
  chprintf(chp, "Version         %8d\r\n", header->version);
  chprintf(chp, "Initial layer 0x%08x\r\n",
           flash_storage_get_pointer_from_idx(header->initial_layer_idx));
  chprintf(chp, "First layer   0x%08x\r\n",
           flash_storage_get_pointer_from_idx(header->first_layer_idx));
  chprintf(chp, "Display    0   1   2   3   4   5   6   7   8\r\n");
  chprintf(chp, "Contrast ");
  uint8_t display = 0;
  uint8_t *initial_contrast = ((flash_storage_header_t *)_flash_storage_area)->display_contrast;
  for (display = 0; display < GLCD_DISP_MAX; display++)
  {
    chprintf(chp, "%3d ", initial_contrast[display]);
  }
  chprintf(chp, "\r\n");
}

void flash_storage_write_default_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;

  if (argc != 0)
  {
    chprintf(chp, "Usage: fs-write-default name\r\n");
    return;
  }
  chprintf(chp, "Writing default config to flash...");
  _flash_storage_write_default_config();
  chprintf(chp, "verify...");
  if (_flash_storage_verify_config((uint8_t *)&_flash_storage_default_layer,
                                   sizeof(_flash_storage_default_layer)))
  {
    chprintf(chp, "done!\r\n");
  }
  else
  {
    chprintf(chp, "abort!\r\n");
  }
}
#endif

/*
 * API functions
 */
void flash_storage_init(void)
{
  _flash_storage_init_hal();
  _flash_storage_init_module();
}

void *flash_storage_get_pointer_from_offset(uint32_t offset)
{
  /*
   * Use array offset to calculate absolute address
   */
  return (void *)&_flash_storage_area[offset];
}

void *flash_storage_get_pointer_from_idx(uint32_t idx)
{
  /*
   * Return absoulte address for idx != 0
   */
  return (idx) ? flash_storage_get_pointer_from_offset(idx) : NULL;
}

anykey_layer_t *flash_storage_get_initial_layer(void)
{
  /*
   * Return absoulte pointer to initial layer
   */
  return flash_storage_get_pointer_from_idx(
      ((flash_storage_header_t *)_flash_storage_area)->initial_layer_idx);
}

anykey_layer_t *flash_storage_get_first_layer(void)
{
  /*
   * Return absoulte pointer to first layer in linked list
   */
  return flash_storage_get_pointer_from_idx(
      ((flash_storage_header_t *)_flash_storage_area)->first_layer_idx);
}

anykey_layer_t *flash_storage_get_layer_by_name(char *name)
{
  /*
   * Search iteratively for layer name in linked list
   * and return absolute pointer to layer if found.
   * Otherwise return NULL.
   */
  anykey_layer_t *layer = flash_storage_get_first_layer();
  while (layer)
  {
    char *layer_name = flash_storage_get_pointer_from_idx(layer->name_idx);
    if (strcmp((const char *)name, (const char *)layer_name) == 0)
    {
      return layer;
    }
    layer = flash_storage_get_pointer_from_idx(layer->next_idx);
  }
  return NULL;
}

void flash_storage_get_display_contrast(uint8_t *contrast_buffer)
{
  /*
   * Return contrast values
   */
  if (contrast_buffer)
  {
    flash_storage_header_t *header = (flash_storage_header_t *)_flash_storage_area;
    memcpy(contrast_buffer, header->display_contrast, sizeof(uint8_t) * ANYKEY_NUMBER_OF_KEYS);
  }
}

void flash_storage_write_sector(uint8_t *buffer, uint16_t sector)
{
  uint32_t wait_time = 0;
  uint16_t start_sector = _flash_storage_get_first_sector();

  /*
   * Erase selected sector and write afterwards
   */
  efl_lld_start_erase_sector(&FLASH_STORAGE_DRIVER_HANDLE, (flash_sector_t)(start_sector + sector));
  efl_lld_query_erase(&FLASH_STORAGE_DRIVER_HANDLE, &wait_time);
  chThdSleep(TIME_MS2I(wait_time));

  efl_lld_program(&FLASH_STORAGE_DRIVER_HANDLE, sector * STM32_FLASH_SECTOR_SIZE,
                  STM32_FLASH_SECTOR_SIZE, (const uint8_t *)buffer);
}
