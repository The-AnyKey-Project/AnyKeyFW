/*
 * glcd_cfg.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CFG_HAL_GLCD_CFG_H_
#define INC_CFG_HAL_GLCD_CFG_H_

#define GLCD_SCK_LINE            PAL_LINE(GPIOA, 5U)
#define GLCD_MOSI_LINE           PAL_LINE(GPIOA, 7U)
#define GLCD_DC_LINE             PAL_LINE(GPIOA, 2U)
#define GLCD_RESET_LINE          PAL_LINE(GPIOA, 1U)

#define GLCD_CS_LINE_1           PAL_LINE(GPIOB, 5U)
#define GLCD_CS_LINE_2           PAL_LINE(GPIOA, 4U)
#define GLCD_CS_LINE_3           PAL_LINE(GPIOB, 13U)
#define GLCD_CS_LINE_4           PAL_LINE(GPIOB, 4U)
#define GLCD_CS_LINE_5           PAL_LINE(GPIOA, 6U)
#define GLCD_CS_LINE_6           PAL_LINE(GPIOB, 14U)
#define GLCD_CS_LINE_7           PAL_LINE(GPIOB, 3U)
#define GLCD_CS_LINE_8           PAL_LINE(GPIOB, 1U)
#define GLCD_CS_LINE_9           PAL_LINE(GPIOB, 15U)

#define GLCD_SPI_DRIVER          (&SPID1)
#define GLCD_SPI_CR1_MODE        (0)                // Mode 3
#define GLCD_SPI_CR1_BR          (SPI_CR1_BR_1)     // ~8MHz
#define GLCD_SPI_BUFFER_SIZE     512

#define GLCD_UPDATE_THREAD_PRIO  (NORMALPRIO)
#define GLCD_UPDATE_THREAD_STACK 512
#define GLCD_UPDATE_THREAD_P_MS  200

#define GLCD_DISPLAY_BLOCK_SIZE 8
#define GLCD_DISPLAY_WIDTH      64
#define GLCD_DISPLAY_HEIGTH     48

#define GLCD_DEFAULT_BRIGHTNESS 128


/*
 * Derived configuration
 */
#define GLCD_SPI_CR1             (GLCD_SPI_CR1_MODE| GLCD_SPI_CR1_BR)
#define GLCD_SPI_CR2             0

#define GLCD_DISPLAY_BUFFER     ((GLCD_DISPLAY_WIDTH*GLCD_DISPLAY_HEIGTH)/GLCD_DISPLAY_BLOCK_SIZE)

#endif /* INC_CFG_HAL_GLCD_CFG_H_ */
