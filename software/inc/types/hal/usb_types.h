/*
 * usb_types.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_TYPES_HAL_USB_TYPES_H_
#define INC_TYPES_HAL_USB_TYPES_H_

#define USB_DESC_CHAR_PADDED(X) X, '\0'
typedef enum
{
  USB_HID_KBD_INTERFACE = 0,
  USB_HID_KBDEXT_INTERFACE,
  USB_HID_RAW_INTERFACE,
  USB_CDC_INT_INTERFACE,
  USB_CDC_DATA_INTERFACE,
  USB_NUM_INTERFACES
} usb_interface_id_t;

typedef enum
{
  USB_HID_KBD_EP = 1,
  USB_HID_KBDEXT_EP,
  USB_HID_RAW_EP,
  USB_CDC_INT_REQUEST_EP,
  USB_CDC_DATA_REQUEST_EP,
  USB_CDC_DATA_AVAILABLE_EP = USB_CDC_DATA_REQUEST_EP
} usb_ep_id_t;

typedef enum
{
  USB_HID_KBD_EPS_IN = 0,
  USB_HID_KBDEXT_EPS_IN,
  USB_HID_RAW_EPS_IN,
  USB_CDC_INT_REQUEST_EPS_IN,
  USB_CDC_DATA_REQUEST_EPS_IN,
  USB_NUM_IN_EPS
} usb_ep_in_state_id_t;

typedef enum
{
  USB_HID_RAW_EPS_OUT = 0,
  USB_CDC_DATA_AVAILABLE_EPS_OUT,
  USB_NUM_OUT_EPS
} usb_ep_out_state_id_t;

#define USB_CALLBACK_STUB(fname)          \
  void fname(USBDriver *usbp, usbep_t ep) \
  {                                       \
    (void)usbp;                           \
    (void)ep;                             \
  }

#endif /* INC_TYPES_HAL_USB_TYPES_H_ */
