/*
 * usb_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_HAL_USB_CFG_H_
#define INC_CFG_HAL_USB_CFG_H_

#define USB_DRIVER_HANDLE     USBD1
#define USB_CDC_DRIVER_HANDLE SDU1

#define USB_VENDOR_ID  0xFEED
#define USB_PRODUCT_ID 0xBABE
#define USB_DEVICE_VER 0x0100

#define USB_HID_KBD_EPSIZE    8
#define USB_HID_KBDEXT_EPSIZE 8
#define USB_HID_RAW_EPSIZE    16

#define USB_CONFIG_DESC_HEADER_SIZE 9
#define USB_CONFIG_DESC_IF_SIZE     9
#define USB_CONFIG_DESC_EP_SIZE     7
#define USB_CONFIG_DESC_HID_SIZE    9
#define USB_CONFIG_DESC_CDC_SIZE    19
#define USB_CONFIG_DESC_IAD_SIZE    8

#define USB_HID_KBD_DESC_SIZE \
  (USB_CONFIG_DESC_IF_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_CONFIG_DESC_EP_SIZE)
#define USB_HID_KBDEXT_DESC_SIZE \
  (USB_CONFIG_DESC_IF_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_CONFIG_DESC_EP_SIZE)
#define USB_HID_RAW_DESC_SIZE \
  (USB_CONFIG_DESC_IF_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_CONFIG_DESC_EP_SIZE)
#define USB_CDC_DESC_SIZE                                                         \
  (USB_CONFIG_DESC_IF_SIZE + USB_CONFIG_DESC_CDC_SIZE + USB_CONFIG_DESC_EP_SIZE + \
   USB_CONFIG_DESC_IF_SIZE + USB_CONFIG_DESC_EP_SIZE + USB_CONFIG_DESC_EP_SIZE)
#define USB_CONFIG_DESC_SIZE                                                        \
  (USB_CONFIG_DESC_HEADER_SIZE + USB_CONFIG_DESC_IAD_SIZE + USB_HID_KBD_DESC_SIZE + \
   USB_HID_KBDEXT_DESC_SIZE + USB_HID_RAW_DESC_SIZE + USB_CONFIG_DESC_IAD_SIZE +    \
   USB_CDC_DESC_SIZE)

#define USB_HID_KBD_DESC_OFFSET \
  (USB_CONFIG_DESC_HEADER_SIZE + USB_CONFIG_DESC_IAD_SIZE + USB_CONFIG_DESC_IF_SIZE)
#define USB_HID_KBDEXT_DESC_OFFSET (USB_HID_KBD_DESC_OFFSET + USB_HID_KBD_DESC_SIZE)
#define USB_HID_RAW_DESC_OFFSET    (USB_HID_KBDEXT_DESC_OFFSET + USB_HID_KBDEXT_DESC_SIZE)
#define USB_CDC_DESC_OFFSET \
  (USB_HID_RAW_DESC_OFFSET + USB_HID_RAW_DESC_SIZE + USB_CONFIG_DESC_IAD_SIZE)

#define USB_DESCRIPTOR_CDC         0x06
#define USB_DESCRIPTOR_HID         0x21
#define USB_DESCRIPTOR_HID_REPORT  0x22
#define USB_HID_REPORT_ID_SYSTEM   2
#define USB_HID_REPORT_ID_CONSUMER 3
#define HID_GET_REPORT             0x01
#define HID_GET_IDLE               0x02
#define HID_GET_PROTOCOL           0x03
#define HID_SET_REPORT             0x09
#define HID_SET_IDLE               0x0A
#define HID_SET_PROTOCOL           0x0B

#endif /* INC_CFG_HAL_USB_CFG_H_ */
