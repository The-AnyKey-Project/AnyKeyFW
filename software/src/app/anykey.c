/*
 * anykey.c
 *
 *  Created on: 08.01.2021
 *      Author: matti
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
#include "api/app/anykey.h"

/*
 * Include dependencies
 */
#include "api/app/cmd_shell.h"
#include "api/hal/flash_storage.h"
#include "api/hal/glcd.h"
#include "api/hal/keypad.h"
#include "api/hal/usb.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
 * Static asserts
 */
static_assert(ANYKEY_NUMBER_OF_KEYS == (KEYPAD_SW_COUNT),
              "Number of keys does not match number used keypad switches");
static_assert(ANYKEY_NUMBER_OF_KEYS == (GLCD_DISP_MAX),
              "Number of keys does not match number used displays");

/*
 * Forward declarations of static functions
 */
static void _anykey_init_hal(void);
static void _anykey_init_module(void);
static void _anykey_fill_response_buffer(uint8_t *buffer, uint16_t already_filled, uint16_t size);
static anykey_layer_t *_anykey_get_layer_by_name(char *search_name);
static void _anykey_set_layer(anykey_layer_t *layer);
static void _anykey_handle_action(anykey_action_list_t *action_list);
static void _anykey_show_actions(BaseSequentialStream *chp, anykey_action_list_t *action_list);

/*
 * Static variables
 */
static THD_WORKING_AREA(_anykey_key_stack, ANYKEY_KEY_THREAD_STACK);
static THD_WORKING_AREA(_anykey_cmd_stack, ANYKEY_CMD_THREAD_STACK);
static anykey_layer_t *_anykey_current_layer = (anykey_layer_t *)NULL;

/*
 * Global variables
 */

/*
 * Tasks
 */

static __attribute__((noreturn)) THD_FUNCTION(_anykey_key_thread, arg)
{
  (void)arg;

  chRegSetThreadName("anykey_key_th");

  event_listener_t event_listener;
  chEvtRegister(&keypad_event_handle, &event_listener, KEYPAD_EVENT_NOTIFIER_BIT);

  while (true)
  {
    eventmask_t events = chEvtWaitAny(EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT));
    if (events & EVENT_MASK(KEYPAD_EVENT_NOTIFIER_BIT))
    {
      keypad_event_t dest[ANYKEY_NUMBER_OF_KEYS];
      keypad_get_sw_events(dest);
      uint8_t sw_id = 0;
      if (_anykey_current_layer)
      {
        for (sw_id = 0; sw_id < ANYKEY_NUMBER_OF_KEYS; sw_id++)
        {
          switch (dest[sw_id])
          {
            case KEYPAD_EVENT_PRESS:
              _anykey_handle_action(flash_storage_get_pointer_from_idx(
                  _anykey_current_layer->key_action_press_idx[sw_id]));
              break;
            case KEYPAD_EVENT_RELEASE:
              _anykey_handle_action(flash_storage_get_pointer_from_idx(
                  _anykey_current_layer->key_action_release_idx[sw_id]));
              break;
            case KEYPAD_EVENT_NONE:
            default:
              break;
          }
        }
      }
    }
  }
}

static __attribute__((noreturn)) THD_FUNCTION(_anykey_cmd_thread, arg)
{
  (void)arg;

  chRegSetThreadName("anykey_cmd_th");

  while (true)
  {
    uint8_t input_buffer[USB_HID_RAW_EPSIZE];
    uint8_t send_resp = 0;
    size_t size = usb_hid_raw_receive(input_buffer, USB_HID_RAW_EPSIZE);
    anykey_cmd_req_t *req = (anykey_cmd_req_t *)input_buffer;
    anykey_cmd_resp_t *resp = (anykey_cmd_resp_t *)input_buffer;
    if (size)
    {
      switch (req->raw.cmd)
      {
        case ANYKEY_CMD_SET_LAYER:
          _anykey_set_layer(flash_storage_get_layer_by_name((char *)req->set_layer.name));
          break;
        case ANYKEY_CMD_GET_LAYER:
          resp->get_layer.name[0] = '\0';
          if (_anykey_current_layer)
          {
            char *name = flash_storage_get_pointer_from_idx(_anykey_current_layer->name_idx);
            if (name)
            {
              strcpy((char *)resp->get_layer.name, (char *)name);
            }
          }
          _anykey_fill_response_buffer((uint8_t *)resp, sizeof(anykey_cmd_get_layer_resp_t),
                                       USB_HID_RAW_EPSIZE);
          send_resp = 1;
          break;
        case ANYKEY_CMD_SET_CONTRAST:
        {
          uint8_t i = 0;
          for (i = 0; i < GLCD_DISP_MAX; i++)
          {
            if (req->set_contrast.display == i || req->set_contrast.display == GLCD_DISP_MAX)
            {
              glcd_set_contrast(i, req->set_contrast.contrast[i]);
            }
          }
          break;
        }
        case ANYKEY_CMD_GET_CONTRAST:
        {
          uint8_t i = 0;
          glcd_display_id_t id = req->get_contrast.display;
          for (i = 0; i < GLCD_DISP_MAX; i++)
          {
            resp->get_contrast.contrast[i] =
                (id == i || id == GLCD_DISP_MAX) ? glcd_get_contrast(i) : 0;
          }
          _anykey_fill_response_buffer((uint8_t *)resp, sizeof(anykey_cmd_get_contrast_resp_t),
                                       USB_HID_RAW_EPSIZE);
          send_resp = 1;
          break;
        }
        case ANYKEY_CMD_GET_FLASH_INFO:
        {
          const flash_descriptor_t *desc = efl_lld_get_descriptor(&FLASH_STORAGE_DRIVER_HANDLE);
          resp->get_flash_info.flash_size = FLASH_STORAGE_SIZE;
          resp->get_flash_info.sector_size = desc->sectors_size;
          _anykey_fill_response_buffer((uint8_t *)resp, sizeof(anykey_cmd_get_flash_info_resp_t),
                                       USB_HID_RAW_EPSIZE);
          send_resp = 1;
          break;
        }
        case ANYKEY_CMD_SET_FALSH:
        {
          static uint8_t _anykey_flash_sector_buffer[STM32_FLASH_SECTOR_SIZE];
          static uint16_t _anykey_flash_sector_written = 0;
          if (req->set_flash.block_cnt == 0)
          {
            memset(_anykey_flash_sector_buffer, 0xff, sizeof(_anykey_flash_sector_buffer));
            _anykey_flash_sector_written = 0;
          }
          memcpy(&_anykey_flash_sector_buffer[_anykey_flash_sector_written], req->set_flash.buffer,
                 req->set_flash.block_size);
          _anykey_flash_sector_written += req->set_flash.block_size;
          if (req->set_flash.final_block)
          {
            flash_storage_write_sector(_anykey_flash_sector_buffer, req->set_flash.sector);
          }
          _anykey_fill_response_buffer((uint8_t *)resp, sizeof(anykey_cmd_set_flash_resp_t),
                                       USB_HID_RAW_EPSIZE);
          send_resp = 1;
          break;
        }
        case ANYKEY_CMD_GET_FALSH:
        {
          const flash_descriptor_t *desc = efl_lld_get_descriptor(&FLASH_STORAGE_DRIVER_HANDLE);
          uint16_t block_size = sizeof(resp->get_flash.buffer);
          uint16_t block_cnt_max = (desc->sectors_size - 1) / block_size + 1;
          uint16_t residual = desc->sectors_size;
          uint8_t *sector = (uint8_t *)flash_storage_get_pointer_from_offset(req->get_flash.sector *
                                                                             desc->sectors_size);
          for (resp->get_flash.block_cnt = 0; resp->get_flash.block_cnt < block_cnt_max;
               resp->get_flash.block_cnt++)
          {
            resp->get_flash.block_size = (residual > block_size) ? block_size : residual;
            residual -= resp->get_flash.block_size;
            resp->get_flash.final_block = (residual) ? 0 : 1;
            memcpy(resp->get_flash.buffer, &sector[block_size * resp->get_flash.block_cnt],
                   resp->get_flash.block_size);
            usb_hid_raw_send((uint8_t *)resp, USB_HID_RAW_EPSIZE);
          }

          break;
        }
        default:
          break;
      }
      if (send_resp)
      {
        usb_hid_raw_send((uint8_t *)resp, USB_HID_RAW_EPSIZE);
      }
    }
  }
}

/*
 * Static helper functions
 */

static void _anykey_init_hal(void)
{
#if defined(USE_STLINK)
  AFIO->MAPR |= (2 << 24);  // Disable NJTRST, allow SWD
#else
  AFIO->MAPR |= (4 << 24);  // Disable SWD, use all pins as GPIO
#endif
}

static void _anykey_init_module(void)
{
  _anykey_set_layer(flash_storage_get_initial_layer());

  chThdCreateStatic(_anykey_key_stack, sizeof(_anykey_key_stack), ANYKEY_KEY_THREAD_PRIO,
                    _anykey_key_thread, NULL);
  chThdCreateStatic(_anykey_cmd_stack, sizeof(_anykey_cmd_stack), ANYKEY_CMD_THREAD_PRIO,
                    _anykey_cmd_thread, NULL);
}

static void _anykey_fill_response_buffer(uint8_t *buffer, uint16_t already_filled, uint16_t size)
{
  memset(&buffer[already_filled], 0, size - already_filled);
}

static anykey_layer_t *_anykey_get_layer_by_name(char *search_name)
{
  anykey_layer_t *layer = flash_storage_get_first_layer();
  while (layer)
  {
    uint8_t *name = flash_storage_get_pointer_from_idx(layer->name_idx);
    if (name)
    {
      if (strcmp((const char *)search_name, (const char *)name) == 0) return layer;
    }
    layer = flash_storage_get_pointer_from_idx(layer->next_idx);
  }
  return layer;
}

static void _anykey_set_layer(anykey_layer_t *layer)
{
  if (layer)
  {
    chSysLock();
    _anykey_current_layer = layer;
    chSysUnlock();
    glcd_set_displays(layer->display_idx);
  }
}

static void _anykey_handle_action(anykey_action_list_t *action_list)
{
  uint8_t i = 0;
  if (action_list)
  {
    while (i < action_list->length)
    {
      switch (action_list->actions[i])
      {
        case ANYKEY_ACTION_KEY_PRESS:
        {
          anykey_action_key_t *action = (anykey_action_key_t *)&(action_list->actions[i]);
          usb_hid_kbd_send_key(action->mods, action->key);
          i += sizeof(anykey_action_key_t);
          break;
        }
        case ANYKEY_ACTION_KEYEXT_PRESS:
        {
          anykey_action_keyext_t *action = (anykey_action_keyext_t *)&(action_list->actions[i]);
          usb_hid_kbdext_send_key(action->report_id, action->key);
          i += sizeof(anykey_action_keyext_t);
          break;
        }
        case ANYKEY_ACTION_KEY_RELEASE:
        {
          anykey_action_key_t *action = (anykey_action_key_t *)&(action_list->actions[i]);
          usb_hid_kbd_send_key(action->mods, action->key | 0x80);
          i += sizeof(anykey_action_key_t);
          break;
        }
        case ANYKEY_ACTION_KEYEXT_RELEASE:
        {
          anykey_action_keyext_t *action = (anykey_action_keyext_t *)&(action_list->actions[i]);
          usb_hid_kbdext_send_key(action->report_id, action->key | 0x8000);
          i += sizeof(anykey_action_keyext_t);
          break;
        }
        case ANYKEY_ACTION_NEXT_LAYER:
          // no parameter -> no cast needed
          _anykey_set_layer(flash_storage_get_pointer_from_idx(_anykey_current_layer->next_idx));
          i += sizeof(anykey_action_layer_t);
          break;
        case ANYKEY_ACTION_PREV_LAYER:
          // no parameter -> no cast needed
          _anykey_set_layer(flash_storage_get_pointer_from_idx(_anykey_current_layer->prev_idx));
          i += sizeof(anykey_action_layer_t);
          break;
        case ANYKEY_ACTION_ADJUST_CONTRAST:
        {
          anykey_action_contrast_t *action = (anykey_action_contrast_t *)&(action_list->actions[i]);
          uint8_t sw_id = 0;
          for (sw_id = 0; sw_id < ANYKEY_NUMBER_OF_KEYS; sw_id++)
          {
            int16_t new_value = (int16_t)glcd_get_contrast(sw_id);
            new_value += action->adjust;
            new_value = (new_value > 255) ? 255 : ((new_value < 0) ? 0 : new_value);
            glcd_set_contrast((glcd_display_id_t)sw_id, (uint8_t)new_value);
          }
          i += sizeof(anykey_action_contrast_t);
        }
        break;
        default:
          i++;
          break;
      }
    }
  }
}

static void _anykey_show_actions(BaseSequentialStream *chp, anykey_action_list_t *action_list)
{
  uint8_t i = 0;
  if (action_list)
  {
    chprintf(chp, "Decoding list with %d bytes length\r\n", action_list->length);
    chprintf(chp, "%-16s %-16s Raw\r\n", "Action", "Operators");
    while (i < action_list->length &&
           chnGetTimeout((BaseChannel *)chp, TIME_IMMEDIATE) == Q_TIMEOUT)
    {
      uint8_t raw_length = 0;
      char action_name[16];
      char operators[16];
      switch (action_list->actions[i])
      {
        case ANYKEY_ACTION_KEY_PRESS:
        {
          anykey_action_key_t *action = (anykey_action_key_t *)&(action_list->actions[i]);
          chsnprintf(action_name, sizeof(action_name), "%s", "KEY_PRESS");
          chsnprintf(operators, sizeof(operators), "0x%02x 0x%02x", action->mods, action->key);
          raw_length = sizeof(anykey_action_key_t);
          break;
        }
        case ANYKEY_ACTION_KEYEXT_PRESS:
        {
          anykey_action_keyext_t *action = (anykey_action_keyext_t *)&(action_list->actions[i]);
          chsnprintf(action_name, sizeof(action_name), "%s", "KEYEXT_PRESS");
          chsnprintf(operators, sizeof(operators), "0x%02x 0x%04x", action->report_id, action->key);
          raw_length = sizeof(anykey_action_keyext_t);
          break;
        }
        case ANYKEY_ACTION_KEY_RELEASE:
        {
          anykey_action_key_t *action = (anykey_action_key_t *)&(action_list->actions[i]);
          chsnprintf(action_name, sizeof(action_name), "%s", "KEY_RELEASE");
          chsnprintf(operators, sizeof(operators), "0x%02x 0x%02x", action->mods, action->key);
          raw_length = sizeof(anykey_action_key_t);
          break;
        }
        case ANYKEY_ACTION_KEYEXT_RELEASE:
        {
          anykey_action_keyext_t *action = (anykey_action_keyext_t *)&(action_list->actions[i]);
          chsnprintf(action_name, sizeof(action_name), "%s", "KEYEXT_RELEASE");
          chsnprintf(operators, sizeof(operators), "0x%02x 0x%04x", action->report_id, action->key);
          raw_length = sizeof(anykey_action_keyext_t);
          break;
        }
        case ANYKEY_ACTION_NEXT_LAYER:
          chsnprintf(action_name, sizeof(action_name), "%s", "NEXT_LAYER");
          operators[0] = '\0';
          raw_length = sizeof(anykey_action_layer_t);
          break;
        case ANYKEY_ACTION_PREV_LAYER:
          chsnprintf(action_name, sizeof(action_name), "%s", "PREV_LAYER");
          operators[0] = '\0';
          raw_length = sizeof(anykey_action_layer_t);
          break;
        case ANYKEY_ACTION_ADJUST_CONTRAST:
        {
          anykey_action_contrast_t *action = (anykey_action_contrast_t *)&(action_list->actions[i]);
          chsnprintf(action_name, sizeof(action_name), "%s", "ADJUST_CONTRAST");
          chsnprintf(operators, sizeof(operators), "%4d", action->adjust);
          raw_length = sizeof(anykey_action_contrast_t);
        }
        break;
        default:
          raw_length = 0;
          i++;
          break;
      }
      if (raw_length)
      {
        chprintf(chp, "%-16s %-16s ", action_name, operators);
        uint8_t x = 0;

        for (x = 0; x < raw_length; x++)
        {
          chprintf(chp, "0x%02x ", action_list->actions[i + x]);
        }

        chprintf(chp, "\r\n");
        i += raw_length;
      }
    }
  }
}

/*
 * Callback functions
 */

/*
 * Shell functions
 */

void anykey_show_actions_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 1)
  {
    chprintf(chp, "Usage: ak-show-actions address\r\n");
    return;
  }
  anykey_action_list_t *action_list = (anykey_action_list_t *)strtol(argv[0], NULL, 16);
  chprintf(chp, "Trying to decode action list at address 0x%08p\r\n\r\n", action_list);
  _anykey_show_actions(chp, action_list);
}

void anykey_show_layer_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  if (argc != 1)
  {
    chprintf(chp, "Usage: ak-show-layer name\r\n");
    return;
  }
  anykey_layer_t *layer = _anykey_get_layer_by_name(argv[0]);
  if (layer == NULL)
  {
    chprintf(chp, "Can't find layer!\r\n");
    return;
  }
  uint8_t active = (layer == _anykey_current_layer) ? 'x' : ' ';
  void *next = flash_storage_get_pointer_from_idx(layer->next_idx);
  void *prev = flash_storage_get_pointer_from_idx(layer->prev_idx);
  uint8_t i = 0;
  chprintf(chp, "Found layer %s at address 0x%08p\r\n\r\n", argv[0], layer);
  chprintf(chp, " Active Next        Prev\r\n");
  chprintf(chp, "   %c    0x%08p  0x%08p\r\n\r\n", active, next, prev);

  chprintf(chp, "          |");
  for (i = 0; i < ANYKEY_NUMBER_OF_KEYS; i++)
  {
    chprintf(chp, "     %d     ", i);
  }

  chprintf(chp, "\r\n Displays |");
  for (i = 0; i < ANYKEY_NUMBER_OF_KEYS; i++)
  {
    void *p = flash_storage_get_pointer_from_idx(layer->display_idx[i]);
    chprintf(chp, " 0x%08p", p);
  }
  chprintf(chp, "\r\n Press    |");
  for (i = 0; i < ANYKEY_NUMBER_OF_KEYS; i++)
  {
    void *p = flash_storage_get_pointer_from_idx(layer->key_action_press_idx[i]);
    chprintf(chp, " 0x%08p", p);
  }
  chprintf(chp, "\r\n Release  |");
  for (i = 0; i < ANYKEY_NUMBER_OF_KEYS; i++)
  {
    void *p = flash_storage_get_pointer_from_idx(layer->key_action_release_idx[i]);
    chprintf(chp, " 0x%08p", p);
  }
  chprintf(chp, "\r\n");
}

void anykey_list_layers_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  if (argc > 0)
  {
    chprintf(chp, "Usage: ak-list-layers\r\n");
    return;
  }
  chprintf(chp, " Active Name\r\n");
  anykey_layer_t *layer = flash_storage_get_first_layer();
  while (layer)
  {
    uint8_t active = (layer == _anykey_current_layer) ? 'x' : ' ';
    uint8_t *name = flash_storage_get_pointer_from_idx(layer->name_idx);
    uint8_t empty[] = "---\0";
    chprintf(chp, "   %c    %s\r\n", active, ((name) ? name : empty));
    layer = flash_storage_get_pointer_from_idx(layer->next_idx);
  }
}

void anykey_set_layer_sh(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  if (argc != 1)
  {
    chprintf(chp, "Usage: ak-set-layer name\r\n");
    return;
  }
  anykey_layer_t *layer = _anykey_get_layer_by_name(argv[0]);
  if (layer == NULL)
  {
    chprintf(chp, "Can't find layer!\r\n");
    return;
  }

  if (layer != _anykey_current_layer)
  {
    chprintf(chp, "Activating layer %s\r\n", argv[0]);
    _anykey_set_layer(layer);
  }
  else
  {
    chprintf(chp, "Layer %s is already active\r\n", argv[0]);
  }
}

/*
 * API functions
 */

void anykey_init(void)
{
  /*
   * Toplevel hal init
   * (disable jtag functionality)
   */
  _anykey_init_hal();

  /*
   * Project specific hal initialization
   */
  flash_storage_init();
  glcd_init();
  keypad_init();
  usb_init();

#if defined(USE_CMD_SHELL)
  /*
   * Initialize additional application components
   */
  cmd_shell_init();
#endif

  /*
   * Initialize toplevel application
   */
  _anykey_init_module();

#if defined(USE_CMD_SHELL)
  /*
   * Start shell handling,
   * should never return
   */
  cmd_shell_loop();
#else
  /*
   * Nothing to do, kill initial thread
   */
  chThdExit("good bye");
#endif
  /*
   * If we reach this point,
   * something went horribly wrong
   */
}
