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
extern void anykey_loop_keys(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_show_layer(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_list_layers(BaseSequentialStream *chp, int argc, char *argv[]);
extern void anykey_set_layer(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module anykey
 */
#define ANYKEY_CMD_LIST \
            {"ak-loop-keys",   anykey_loop_keys}, \
            {"ak-show-layer",  anykey_show_layer}, \
            {"ak-list-layers", anykey_list_layers}, \
            {"ak-set-layer",   anykey_set_layer}, \

#endif /* INC_CMD_ANYKEY_CMD_H_ */
