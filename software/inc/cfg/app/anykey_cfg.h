/*
 * anykey_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_APP_ANYKEY_CFG_H_
#define INC_CFG_APP_ANYKEY_CFG_H_

#define ANYKEY_NUMBER_OF_KEYS 9

#define ANYKEY_KEY_THREAD_STACK 256
#define ANYKEY_KEY_THREAD_PRIO  (NORMALPRIO - 2)

#define ANYKEY_CMD_THREAD_STACK 256
#define ANYKEY_CMD_THREAD_PRIO  (NORMALPRIO - 1)

#endif /* INC_CFG_APP_ANYKEY_CFG_H_ */
