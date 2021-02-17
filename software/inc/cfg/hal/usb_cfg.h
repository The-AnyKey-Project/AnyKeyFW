/*
 * usb_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_HAL_USB_CFG_H_
#define INC_CFG_HAL_USB_CFG_H_

#define USB_DRIVER_HANDLE          USBD1
#define USB_CDC_DRIVER_HANDLE      SDU1
#define USB_VENDOR_ID              0xFEED
#define USB_PRODUCT_ID             0xBABE
#define USB_DEVICE_VER             0x0100
#define USB_HID_KBD_EPSIZE         8
#define USB_HID_KBDEXT_EPSIZE      8
#define USB_HID_RAW_EPSIZE         16
#define USB_CONFIG_DESC_HID_SIZE   9
#define USB_CONFIG_DESC_CDC_SIZE   19
#define USB_DESCRIPTOR_CDC         0x24
#define USB_DESCRIPTOR_HID         0x21
#define USB_DESCRIPTOR_HID_REPORT  0x22
#define USB_HID_REPORT_ID_SYSTEM   0x02
#define USB_HID_REPORT_ID_CONSUMER 0x03
#define USB_HID_GET_REPORT         0x01
#define USB_HID_GET_IDLE           0x02
#define USB_HID_GET_PROTOCOL       0x03
#define USB_HID_SET_REPORT         0x09
#define USB_HID_SET_IDLE           0x0A
#define USB_HID_SET_PROTOCOL       0x0B

#define USB_HID_KBD_DESC_DATA_SIZE \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_DESC_ENDPOINT_SIZE)
#define USB_HID_KBDEXT_DESC_DATA_SIZE \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_DESC_ENDPOINT_SIZE)
#define USB_HID_RAW_DESC_DATA_SIZE                                               \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_HID_SIZE + USB_DESC_ENDPOINT_SIZE + \
   USB_DESC_ENDPOINT_SIZE)
#define USB_CDC_DESC_DATA_SIZE                                                   \
  (USB_DESC_INTERFACE_SIZE + USB_CONFIG_DESC_CDC_SIZE + USB_DESC_ENDPOINT_SIZE + \
   USB_DESC_INTERFACE_SIZE + USB_DESC_ENDPOINT_SIZE + USB_DESC_ENDPOINT_SIZE)
#define USB_CONFIG_DESC_DATA_SIZE                                                            \
  (USB_DESC_CONFIGURATION_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE +                       \
   USB_HID_KBD_DESC_DATA_SIZE + USB_HID_KBDEXT_DESC_DATA_SIZE + USB_HID_RAW_DESC_DATA_SIZE + \
   USB_DESC_INTERFACE_ASSOCIATION_SIZE + USB_CDC_DESC_DATA_SIZE)
#define USB_HID_KBD_DESC_OFFSET \
  (USB_DESC_CONFIGURATION_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE + USB_DESC_INTERFACE_SIZE)
#define USB_HID_KBDEXT_DESC_OFFSET (USB_HID_KBD_DESC_OFFSET + USB_HID_KBD_DESC_DATA_SIZE)
#define USB_HID_RAW_DESC_OFFSET    (USB_HID_KBDEXT_DESC_OFFSET + USB_HID_KBDEXT_DESC_DATA_SIZE)
#define USB_CDC_DESC_OFFSET \
  (USB_HID_RAW_DESC_OFFSET + USB_HID_RAW_DESC_DATA_SIZE + USB_DESC_INTERFACE_ASSOCIATION_SIZE)

#endif /* INC_CFG_HAL_USB_CFG_H_ */
