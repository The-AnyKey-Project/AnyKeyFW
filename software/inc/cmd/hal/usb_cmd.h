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
 * usb_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matthiasb85
 */

#ifndef INC_CMD_USB_HID_H_
#define INC_CMD_USB_HID_H_

#if defined(USE_CMD_SHELL)
/*
 * Global definition of shell commands
 * for module keypad
 */
extern void usb_loop_hid_raw_input(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module usb
 */
// clang-format off
#define USB_CMD_LIST \
    {"usb-loop-hid-raw", usb_loop_hid_raw_input}
// clang-format on
#endif

#endif /* INC_CMD_USB_HID_H_ */
