/*
 * flash_storage_cmd.h
 *
 *  Created on: 04.03.2021
 *      Author: matti
 */

#ifndef INC_CMD_HAL_FLASH_STORAGE_CMD_H_
#define INC_CMD_HAL_FLASH_STORAGE_CMD_H_

/*
 * Global definition of shell commands
 * for module flash storage
 */
extern void flash_storage_info_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module anykey
 */
// clang-format off
#define FLASH_STORAGE_CMD_LIST \
            {"flash-storage-info-sh",   flash_storage_info_sh} \
// clang-format on
#endif /* INC_CMD_HAL_FLASH_STORAGE_CMD_H_ */
