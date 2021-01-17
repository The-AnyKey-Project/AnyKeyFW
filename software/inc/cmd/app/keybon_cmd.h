/*
 * keybon_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CMD_KEYBON_CMD_H_
#define INC_CMD_KEYBON_CMD_H_

/*
 * Global definition of shell commands
 * for module keybon
 */
extern void keybon_loop_keys(BaseSequentialStream *chp, int argc, char *argv[]);
extern void keybon_show_layer(BaseSequentialStream *chp, int argc, char *argv[]);
extern void keybon_list_layers(BaseSequentialStream *chp, int argc, char *argv[]);
extern void keybon_set_layer(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module keybon
 */
#define KEYBON_CMD_LIST \
            {"kb-loop-keys", keybon_loop_keys}, \
            {"kb-show-layer", keybon_show_layer}, \
            {"kb-list-layers", keybon_list_layers}, \
            {"kb-set-layer", keybon_set_layer}, \

#endif /* INC_CMD_KEYBON_CMD_H_ */
