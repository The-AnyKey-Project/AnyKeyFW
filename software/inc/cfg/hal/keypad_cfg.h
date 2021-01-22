/*
 * keypad_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_HAL_KEYPAD_CFG_H_
#define INC_CFG_HAL_KEYPAD_CFG_H_

#define KEYPAD_POLL_THREAD_STACK        512
#define KEYPAD_POLL_THREAD_PRIO         (NORMALPRIO + 1)
#define KEYPAD_POLL_MAIN_THREAD_P_MS    10

#define KEYPAD_EVENT_NOTIFIER_BIT       0

#define KEYPAD_BTN_MODE                 PAL_MODE_INPUT_PULLUP
#define KEYPAD_BTN_LINE_SW01            PAL_LINE(GPIOA, 3U)
#define KEYPAD_BTN_LINE_SW02            PAL_LINE(GPIOB, 0U)
#define KEYPAD_BTN_LINE_SW03            PAL_LINE(GPIOB, 12U)
#define KEYPAD_BTN_LINE_SW04            PAL_LINE(GPIOA, 15U)
#define KEYPAD_BTN_LINE_SW05            PAL_LINE(GPIOB, 11U)
#define KEYPAD_BTN_LINE_SW06            PAL_LINE(GPIOA, 9U)
#define KEYPAD_BTN_LINE_SW07            PAL_LINE(GPIOA, 14U)
#define KEYPAD_BTN_LINE_SW08            PAL_LINE(GPIOB, 10U)
#define KEYPAD_BTN_LINE_SW09            PAL_LINE(GPIOA, 8U)

#define KEYPAD_BTN_PRESS_DELAY_TIME_MS      1000
#define KEYPAD_BTN_REPRESS_DELAY_TIME_MS    50

/*
 * Derived configuration
 */
#if KEYPAD_BTN_MODE == PAL_MODE_INPUT_PULLUP
#define KEYPAD_BTN_PRESSED              (0)
#define KEYPAD_BTN_UNPRESSED            (1)
#elif KEYPAD_BTN_MODE == PAL_MODE_INPUT_PULLDOWN
#define KEYPAD_BTN_PRESSED              (1)
#define KEYPAD_BTN_UNPRESSED            (0)
#else
#warning "KEYPAD_BTN_PRESSED can't be derived by KEYPAD_BTN_MODE, please specify manually"
#define KEYPAD_BTN_PRESSED              (1)
#define KEYPAD_BTN_UNPRESSED            (0)
#endif

#define KEYPAD_BTN_PRESS_DELAY_TIME_TICKS   (KEYPAD_BTN_PRESS_DELAY_TIME_MS/KEYPAD_POLL_MAIN_THREAD_P_MS)
#define KEYPAD_BTN_REPRESS_DELAY_TIME_TICKS (KEYPAD_BTN_REPRESS_DELAY_TIME_MS/KEYPAD_POLL_MAIN_THREAD_P_MS)

#endif /* INC_CFG_HAL_KEYPAD_CFG_H_ */