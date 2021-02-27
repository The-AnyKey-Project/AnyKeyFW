/*
 * usb_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CMD_USB_HID_H_
#define INC_CMD_USB_HID_H_

/*
 * Global definition of shell commands
 * for module keypad
 */
extern void usb_loop_hid_raw_input(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module kepad
 */
// clang-format off
#define USB_CMD_LIST \
    {"usb-loop-hid-raw", usb_loop_hid_raw_input}
// clang-format on

#endif /* INC_CMD_USB_HID_H_ */
