/*
 * keybon_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_APP_KEYBON_CFG_H_
#define INC_CFG_APP_KEYBON_CFG_H_

#define KEYBON_NUMBER_OF_KEYS    9

#define KEYBON_KEY_THREAD_STACK  512
#define KEYBON_KEY_THREAD_PRIO   (NORMALPRIO - 2)

#define KEYBON_CMD_THREAD_STACK  512
#define KEYBON_CMD_THREAD_PRIO   (NORMALPRIO - 1)

#endif /* INC_CFG_APP_KEYBON_CFG_H_ */
