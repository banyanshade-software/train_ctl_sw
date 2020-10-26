/*
 * cli.h
 *
 *  Created on: Sep 15, 2020
 *      Author: danielbraun
 */

#ifndef SRC_CLI_H_
#define SRC_CLI_H_

#error obsolete

#if 0
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"


void vRegisterCLICommands(void);


extern int cli_frame_mode;

/*
 * frame mode is targeted to be easily and safely processed by machine
 * Frames are delimited by '|'
 * command format :
 * |sSNCvv...|
 * response format :
 * |s'R'rvv..|
 * notif format
 * |s'N'SNCvv..|
 *
 * s : 1byte, seq num to match response. 'z' is reserved
 * S : 1byte, selector : T=train, C=canton, S=switch, G=global ...
 * N : 1byte, train/canton/.. number
 * C : 1byte, train/canton/.. parameter or command
 * vv.. : value(s)
 * r : 1byte response code
 *
 *
 * G : general command
 * 		C	: return to CLI mode
 * 		S   : stop all
 *
 * T : train control
 *
 */

void frame_process(uint8_t c, uint8_t *respbuf, int *replen);
void frame_send_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen);


void frame_console_msg(char *msg);

#endif

#endif /* SRC_CLI_H_ */
