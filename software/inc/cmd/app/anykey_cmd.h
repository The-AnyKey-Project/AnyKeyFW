/*
 * anykey_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CMD_ANYKEY_CMD_H_
#define INC_CMD_ANYKEY_CMD_H_

/*
 * Global definition of shell commands
 * for module anykey
 */
extern void anykey_show_actions(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_show_layer_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_list_layers_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_set_layer_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module anykey
 */
// clang-format off
#define ANYKEY_CMD_LIST \
            {"ak-show-actios", anykey_show_actions}, \
            {"ak-show-layer",  anykey_show_layer_sh}, \
            {"ak-list-layers", anykey_list_layers_sh}, \
            {"ak-set-layer",   anykey_set_layer_sh}
// clang-format on
#endif /* INC_CMD_ANYKEY_CMD_H_ */
