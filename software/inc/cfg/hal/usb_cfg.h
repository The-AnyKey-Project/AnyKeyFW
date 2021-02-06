/*
 * usb_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_HAL_USB_CFG_H_
#define INC_CFG_HAL_USB_CFG_H_

#define USB_SERIAL_DRIVER_HANDLE SDU1

/*
 * Endpoints to be used for USBD1.
 */
#define USBD1_DATA_REQUEST_EP      1
#define USBD1_DATA_AVAILABLE_EP    1
#define USBD1_INTERRUPT_REQUEST_EP 2

#endif /* INC_CFG_HAL_USB_CFG_H_ */
