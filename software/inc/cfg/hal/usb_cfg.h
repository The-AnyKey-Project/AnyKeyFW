/*
 * usb_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_HAL_USB_CFG_H_
#define INC_CFG_HAL_USB_CFG_H_

#define USB_DRIVER_HANDLE USBD1

#define USB_VCOM_INTERFACE            0
#define USB_VCOM_DATA_REQUEST_EP      1
#define USB_VCOM_DATA_AVAILABLE_EP    1
#define USB_VCOM_INTERRUPT_REQUEST_EP 2
#define USB_VCOM_DRIVER_HANDLE        SDU1

#define USB_HID_KBD_INTERFACE 1
#define USB_HID_KBD_EP        3
#define USB_HID_KBD_EP_SIZE   8

#endif /* INC_CFG_HAL_USB_CFG_H_ */
