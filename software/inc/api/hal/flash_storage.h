/*
 * flash_storage.h
 *
 *  Created on: 04.03.2021
 *      Author: matti
 */

#ifndef INC_API_HAL_FLASH_STORAGE_H_
#define INC_API_HAL_FLASH_STORAGE_H_

#include "cfg/hal/flash_storage_cfg.h"
#include "types/hal/flash_storage_types.h"

extern void flash_storage_init(void);
extern void *flash_storage_get_pointer_from_offset(uint32_t offset);
extern anykey_layer_t *flash_storage_get_initial_layer(void);
extern anykey_layer_t *flash_storage_get_first_layer(void);
extern anykey_layer_t *flash_storage_get_layer_by_name(char *name);
extern void flash_storage_write_config(uint32_t offset, uint32_t size, uint8_t *buffer);

#endif /* INC_API_HAL_FLASH_STORAGE_H_ */
