/*
 * keypad_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CMD_KEYPAD_CMD_H_
#define INC_CMD_KEYPAD_CMD_H_

/*
 * Global definition of shell commands
 * for module keypad
 */
extern void keypad_loop_switches_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module kepad
 */
// clang-format off
#define KEYPAD_CMD_LIST \
    {"kp-loop-sw", keypad_loop_switches_sh}
// clang-format on

#endif /* INC_CMD_KEYPAD_CMD_H_ */
