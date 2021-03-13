/*
 * usb_hid.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
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

extern SerialUSBDriver USB_CDC_DRIVER_HANDLE;

#endif /* INC_API_USB_HID_H_ */
