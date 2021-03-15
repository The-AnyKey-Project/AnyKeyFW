/*
 * glcd_cmd.h
 *
 *  Created on: 08.01.2021
 *      Author: matti
 */

#ifndef INC_CMD_GLCD_CMD_H_
#define INC_CMD_GLCD_CMD_H_

/*
 * Global definition of shell commands
 * for module glcd
 */
extern void glcd_set_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void glcd_get_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[]);
extern void glcd_reload_contrast_sh(BaseSequentialStream *chp, int argc, char *argv[]);

/*
 * Shell command list
 * for module glcd
 */
// clang-format off
#define GLCD_CMD_LIST \
    {"glcd-set-contrast",    glcd_set_contrast_sh}, \
    {"glcd-get-contrast",    glcd_get_contrast_sh}, \
    {"glcd-reload-contrast", glcd_reload_contrast_sh}
// clang-format on

#endif /* INC_CMD_GLCD_CMD_H_ */
