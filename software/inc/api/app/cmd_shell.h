/*
 * cmd_shell.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_API_SHELL_H_
#define INC_API_SHELL_H_

#include "cfg/app/cmd_shell_cfg.h"
#include "types/app/cmd_shell_types.h"

extern void cmd_shell_init(void);
extern void cmd_shell_loop(void);

#endif /* INC_API_SHELL_H_ */
