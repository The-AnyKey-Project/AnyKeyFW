/*
 * flash_storage.c
 *
 *  Created on: 04.03.2021
 *      Author: matti
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

/*
 * Static variables
 */
static uint8_t *_flash_storage_area = NULL;
static const flash_storage_default_layer_t _flash_storage_default_layer = {
    .flash_header =
        {
            .crc = 0xa0cc9b21,
            .version = FLASH_STORAGE_HEADER_VERSION,
            .initial_layer_idx = offsetof(flash_storage_default_layer_t, layer_header),
            .first_layer_idx = offsetof(flash_storage_default_layer_t, layer_header),
        },
    .layer_header =
        {
            .next_idx = 0,
            .prev_idx = 0,
            .name_idx = sizeof(anykey_layer_t),
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
                    offsetof(flash_storage_default_layer_t, kp_1),
                    offsetof(flash_storage_default_layer_t, kp_2),
                    offsetof(flash_storage_default_layer_t, kp_3),
                    offsetof(flash_storage_default_layer_t, kp_4),
                    offsetof(flash_storage_default_layer_t, kp_5),
                    offsetof(flash_storage_default_layer_t, kp_6),
                    offsetof(flash_storage_default_layer_t, kp_7),
                    offsetof(flash_storage_default_layer_t, kp_8),
                    offsetof(flash_storage_default_layer_t, kp_9),
                },
            .key_action_release_idx =
                {
                    offsetof(flash_storage_default_layer_t, kr_1),
                    offsetof(flash_storage_default_layer_t, kr_2),
                    offsetof(flash_storage_default_layer_t, kr_3),
                    offsetof(flash_storage_default_layer_t, kr_4),
                    offsetof(flash_storage_default_layer_t, kr_5),
                    offsetof(flash_storage_default_layer_t, kr_6),
                    offsetof(flash_storage_default_layer_t, kr_7),
                    0,
                    0,
                },
        },
    .name = FLASH_STORAGE_DEFCONFIG_NAME,
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
    .kp_1 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, 0xE0)},  // ctrl
            .act_2 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, 0x06)},  // c
        },
    .kp_2 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, 0xE0)},  // ctrl
            .act_2 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, 0x1B)},  // x
        },
    .kp_3 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, 0xE0)},  // ctrl
            .act_2 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_PRESS, 0x19)},  // v
        },
    .kp_4 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_PRESS,
                                                   USB_HID_REPORT_ID_CONSUMER, 0xE2)},  // vol mute
        },
    .kp_5 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_PRESS,
                                                   USB_HID_REPORT_ID_CONSUMER, 0xEA)},  // vol -
        },
    .kp_6 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_PRESS,
                                                   USB_HID_REPORT_ID_CONSUMER, 0xE9)},  // vol +
        },
    .kp_7 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_PRESS,
                                                   USB_HID_REPORT_ID_CONSUMER, 0x0223)},  // browser
        },
    .kp_8 =
        {
            .length = sizeof(anykey_action_contrast_t),
            .act_1 = {FLASH_STORAGE_CONTRAST_CONTENT(ANYKEY_ACTION_ADJUST_CONTRAST,
                                                     -10)},  // dec contrast
        },
    .kp_9 =
        {
            .length = sizeof(anykey_action_contrast_t),
            .act_1 = {FLASH_STORAGE_CONTRAST_CONTENT(ANYKEY_ACTION_ADJUST_CONTRAST,
                                                     10)},  // inc contrast
        },
    .kr_1 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, 0xE0)},  // ctrl
            .act_2 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, 0x06)},  // c
        },
    .kr_2 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, 0xE0)},  // ctrl
            .act_2 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, 0x1B)},  // x
        },
    .kr_3 =
        {
            .length = 2 * sizeof(anykey_action_key_t),
            .act_1 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, 0xE0)},  // ctrl
            .act_2 = {FLASH_STORAGE_KEY_CONTENT(ANYKEY_ACTION_KEY_RELEASE, 0x19)},  // v
        },
    .kr_4 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_RELEASE,
                                                   USB_HID_REPORT_ID_CONSUMER, 0xE2)},  // vol mute
        },
    .kr_5 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_RELEASE,
                                                   USB_HID_REPORT_ID_CONSUMER, 0xEA)},  // vol -
        },
    .kr_6 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_RELEASE,
                                                   USB_HID_REPORT_ID_CONSUMER, 0xE9)},  // vol +
        },
    .kr_7 =
        {
            .length = sizeof(anykey_action_keyext_t),
            .act_1 = {FLASH_STORAGE_KEYEXT_CONTENT(ANYKEY_ACTION_KEYEXT_RELEASE,
                                                   USB_HID_REPORT_ID_CONSUMER, 0x0223)},  // browser
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
  eflStart(&FLASH_STORAGE_DRIVER_HANDLE, NULL);
  rccEnableCRC(true);
  crcStart(&FLASH_STORAGE_CRC_HANDLE, NULL);
}

static void _flash_storage_init_module(void)
{
  _flash_storage_area = (uint8_t *)&__flash1_base__;
  uint32_t crc = crcCalcI(&FLASH_STORAGE_CRC_HANDLE, FLASH_STORAGE_SIZE - sizeof(crc_t),
                          &_flash_storage_area[sizeof(crc_t)]);
  flash_storage_header_t *header = (flash_storage_header_t *)_flash_storage_area;
  if (crc != header->crc)
  {
    _flash_storage_write_default_config();
  }
}

static void _flash_storage_write_default_config(void)
{
  flash_storage_write_config(0, sizeof(flash_storage_default_layer_t),
                             (uint8_t *)&_flash_storage_default_layer);
}

/*
 * Callback functions
 */

/*
 * Shell functions
 */

void flash_storage_info_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)chp;
  (void)argc;
  (void)argv;
}

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
  return (offset) ? (void *)&_flash_storage_area[offset] : NULL;
}

anykey_layer_t *flash_storage_get_initial_layer(void)
{
  return flash_storage_get_pointer_from_offset(
      ((flash_storage_header_t *)_flash_storage_area)->initial_layer_idx);
}

anykey_layer_t *flash_storage_get_first_layer(void)
{
  return flash_storage_get_pointer_from_offset(
      ((flash_storage_header_t *)_flash_storage_area)->first_layer_idx);
}

anykey_layer_t *flash_storage_get_layer_by_name(char *name)
{
  anykey_layer_t *layer = flash_storage_get_first_layer();
  while (layer)
  {
    char *layer_name = flash_storage_get_pointer_from_offset(layer->name_idx);
    if (strcmp((const char *)name, (const char *)layer_name) == 0)
    {
      return layer;
    }
    layer = flash_storage_get_pointer_from_offset(layer->next_idx);
  }
  return NULL;
}

void flash_storage_write_config(uint32_t offset, uint32_t size, uint8_t *buffer)
{
  const flash_descriptor_t *desc = efl_lld_get_descriptor(&FLASH_STORAGE_DRIVER_HANDLE);
  flash_offset_t flash_offset =
      (flash_offset_t)_flash_storage_area - (flash_offset_t)desc->address + (flash_offset_t)offset;

  uint8_t i = 0;
  uint8_t start_sector = FLASH_STORAGE_LAST_SECTOR - FLASH_STORAGE_SIZE / desc->sectors_size;

  for (i = start_sector; i <= FLASH_STORAGE_LAST_SECTOR; i++)
  {
    uint32_t wait_time = 0;
    efl_lld_start_erase_sector(&FLASH_STORAGE_DRIVER_HANDLE, (flash_sector_t)i);
    efl_lld_query_erase(&FLASH_STORAGE_DRIVER_HANDLE, &wait_time);
    chThdSleep(TIME_MS2I(wait_time));
  }

  efl_lld_program(&FLASH_STORAGE_DRIVER_HANDLE, flash_offset, size, (const uint8_t *)buffer);
}
