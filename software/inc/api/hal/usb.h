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
 * usb_hid.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_API_USB_HID_H_
#define INC_API_USB_HID_H_

#include "cfg/hal/usb_cfg.h"
#include "types/hal/usb_types.h"

extern void usb_init(void);
extern void usb_hid_kbd_flush(void);
extern void usb_hid_kbd_send_key(uint8_t mods, uint8_t key);
extern void usb_hid_kbdext_send_key(usb_hid_report_id_t report_id, uint16_t keyext);
extern size_t usb_hid_raw_send(uint8_t *msg, uint8_t size);
extern size_t usb_hid_raw_receive(uint8_t *msg, uint8_t size);

#ifndef HIDRAW_TEST
extern SerialUSBDriver USB_CDC_DRIVER_HANDLE;
#endif

#endif /* INC_API_USB_HID_H_ */
