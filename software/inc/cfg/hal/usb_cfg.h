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
 * usb_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_CFG_HAL_USB_CFG_H_
#define INC_CFG_HAL_USB_CFG_H_

#define USB_DRIVER_HANDLE        USBD1
#define USB_CDC_DRIVER_HANDLE    SDU1
#define USB_VENDOR_ID            0xFEED
#define USB_PRODUCT_ID           0xBABE
#define USB_DEVICE_VER           0x0200
#define USB_HID_KBD_EPSIZE       0x08
#define USB_HID_KBDEXT_EPSIZE    0x08
#define USB_HID_RAW_EPSIZE       0x40
#define USB_CDC_INT_EPSIZE       0x08
#define USB_CDC_DATA_EPSIZE      0x40
#define USB_CONFIG_DESC_HID_SIZE 0x09
#define USB_CONFIG_DESC_CDC_SIZE 0x13
#define USB_CONFIG_HID_CLASS     0x03
#define USB_CONFIG_HID_SUBCLASS  0x01

#define USB_DESCRIPTOR_CDC        CDC_CS_INTERFACE
#define USB_DESCRIPTOR_HID        0x21
#define USB_DESCRIPTOR_HID_REPORT 0x22
#define USB_HID_GET_REPORT        0x01
#define USB_HID_GET_IDLE          0x02
#define USB_HID_GET_PROTOCOL      0x03
#define USB_HID_SET_REPORT        0x09
#define USB_HID_SET_IDLE          0x0A
#define USB_HID_SET_PROTOCOL      0x0B
#define USB_HID_IOF_DATA          (0 << 0)
#define USB_HID_IOF_VARIABLE      (1 << 1)
#define USB_HID_IOF_ABSOLUTE      (0 << 2)
#define USB_HID_IOF_NON_VOLATILE  (0 << 7)

#define USB_HID_KBD_REPORT_POOLSIZE   0x02
#define USB_HID_KBD_REPORT_KEYS       (USB_HID_KBD_EPSIZE - 2)
#define USB_HID_KBD_REPORT_RETRY_MAX  2
#define USB_HID_KBD_REPORT_TIMEOUT_MS 10
#define USB_HID_KBD_REPORT_RETRY_MS   1

#define USB_HID_RAW_INPUT_BUFFER_ENTRIES  2
#define USB_HID_RAW_OUTPUT_BUFFER_ENTRIES 2
#define USB_HID_RAW_IOF_IN                (USB_HID_IOF_DATA | USB_HID_IOF_VARIABLE | USB_HID_IOF_ABSOLUTE)
#define USB_HID_RAW_IOF_OUT \
  (USB_HID_IOF_DATA | USB_HID_IOF_VARIABLE | USB_HID_IOF_ABSOLUTE | USB_HID_IOF_NON_VOLATILE)

#define USB_HID_KBD_DESC_DATA_SIZE \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_DESC_ENDPOINT_SIZE)
#define USB_HID_KBDEXT_DESC_DATA_SIZE \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_DESC_ENDPOINT_SIZE)
#define USB_HID_RAW_DESC_DATA_SIZE                                               \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_DESC_ENDPOINT_SIZE + \
   USB_DESC_ENDPOINT_SIZE)
#if defined(USE_CMD_SHELL)
#define USB_CDC_DESC_DATA_SIZE                                                   \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_CDC_SIZE + USB_DESC_ENDPOINT_SIZE + \
   USB_DESC_INTERFACE_SIZE + USB_DESC_ENDPOINT_SIZE + USB_DESC_ENDPOINT_SIZE)
#else
#define USB_CDC_DESC_DATA_SIZE 0
#endif
#define USB_CONFIG_DESC_DATA_SIZE                                                            \
  (USB_DESC_CONFIGURATION_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE +                       \
   USB_HID_KBD_DESC_DATA_SIZE + USB_HID_KBDEXT_DESC_DATA_SIZE + USB_HID_RAW_DESC_DATA_SIZE + \
   USB_DESC_INTERFACE_ASSOCIATION_SIZE + USB_CDC_DESC_DATA_SIZE)
#define USB_HID_KBD_DESC_OFFSET \
  (USB_DESC_CONFIGURATION_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE + USB_DESC_INTERFACE_SIZE)
#define USB_HID_KBDEXT_DESC_OFFSET (USB_HID_KBD_DESC_OFFSET + USB_HID_KBD_DESC_DATA_SIZE)
#define USB_HID_RAW_DESC_OFFSET    (USB_HID_KBDEXT_DESC_OFFSET + USB_HID_KBDEXT_DESC_DATA_SIZE)

#if defined(USE_CMD_SHELL)
#define USB_CDC_DESC_OFFSET \
  (USB_HID_RAW_DESC_OFFSET + USB_HID_RAW_DESC_DATA_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE)
#endif

#endif /* INC_CFG_HAL_USB_CFG_H_ */
