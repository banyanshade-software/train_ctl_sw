/*
 * cli.c
 *
 *  Created on: Sep 15, 2020
 *      Author: danielbraun
 */

#include <string.h>
#include <stdlib.h>
#include "cli.h"
#include "FreeRTOS_CLI.h"
#include "usbd_cdc_if.h"
#include "trainctl_config.h"

#ifndef TRAINCTL_NEW
#error ho
#endif

#if TRAINCTL_NEW == 0
#include "../../trainctl/trainctl.h"
#else
#include "../../trainctl/traincontrol.h"
#endif
#include "../../trainctl/stm32/txframe.h"
#include "../../trainctl/stm32/taskauto.h"
#include "../../trainctl/param.h"


#define SW_VERSION "0.1"
static portBASE_TYPE get_version_cmd    (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE set_mode_cmd       (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
/*
static portBASE_TYPE stop_cmd           (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE speed_cmd           (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#if TRAINCTL_NEW == 0
static portBASE_TYPE volt_cmd           (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static portBASE_TYPE set_dir_pwm_cmd    (char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#endif
*/

// https://github.com/maxnil/NMEA_Multiplexer_v2.0_FW/blob/2c14dfe3262f477f86e1426f9ec1f923a6e798bc/NMEA-Multiplexer%20v2.0/src/services/CLI-commands.c


#define CLI_DEF_T static const CLI_Command_Definition_t

CLI_DEF_T get_version_cmd_def   = {"version",   "version\r\n",                 get_version_cmd,   0};
CLI_DEF_T set_mode_cmd_def      = {"mode",      "mode cli|frame\r\n",          set_mode_cmd,      1};
/*
CLI_DEF_T stop_cmd_def          = {"stop",      "stop\r\n",                    stop_cmd,          0};
#if TRAINCTL_NEW == 0
CLI_DEF_T volt_cmd_def          = {"volt",      "volt 0-15\r\n",                volt_cmd,          1};
CLI_DEF_T set_dir_pwm_cmd_def   = {"tc",        "tc dir duty_pwm\r\n",         set_dir_pwm_cmd,   2};
#endif
CLI_DEF_T speed_cmd_def         = {"v",         "v [-100:100], set speed\r\n", speed_cmd,         1};
*/


void vRegisterCLICommands(void) {
	FreeRTOS_CLIRegisterCommand(&get_version_cmd_def);
	FreeRTOS_CLIRegisterCommand(&set_mode_cmd_def);
	/*
	FreeRTOS_CLIRegisterCommand(&stop_cmd_def);
	FreeRTOS_CLIRegisterCommand(&speed_cmd_def);
#if TRAINCTL_NEW == 0
	FreeRTOS_CLIRegisterCommand(&volt_cmd_def);
	FreeRTOS_CLIRegisterCommand(&set_dir_pwm_cmd_def);
#endif
*/
}

/*******************************************************************************
 * "Get Version" command
 * Returns SW version
 */
static portBASE_TYPE get_version_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
	//configASSERT(pcWriteBuffer);

	//snprintf(pcWriteBuffer, xWriteBufferLen, "test_usb_f103 %s (%s, %s)\r\n", SW_VERSION, __DATE__, __TIME__);
	strcpy(pcWriteBuffer,  "test_usb_f103 " SW_VERSION " " __DATE__" " __TIME__ "\r\n");
	//sprintf(pcWriteBuffer, "test_usb_f103 %s (xxxxxxxxxxxxxxxxxxxxxxx)\r\n", SW_VERSION);
    //sprintf(pcWriteBuffer, "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
    				   //   0         1         2         3         4         5
	return pdFALSE;
}

int cli_frame_mode=0;

static portBASE_TYPE set_mode_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	portBASE_TYPE p1len = 0;
	const char *p1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &p1len);
	if (!strcmp(p1, "cli")) {
		cli_frame_mode = 0;
	} else if (!strcmp(p1, "frame")) {
		cli_frame_mode = 1;
	} else {
		strcpy(pcWriteBuffer,  "mode should be 'cli' or 'frame'\r\n");
		return pdFALSE;
	}
	strcpy(pcWriteBuffer, "OK\r\n");
	return pdFALSE;
}

/*

static portBASE_TYPE stop_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
#if TRAINCTL_NEW == 0
	train0.speed_target = 0;
	train0.speed_cur = 0;
	trainlow_set_speed(0,0);
	sprintf(pcWriteBuffer, "stopped\r\n");
#else
	train_stop_all();
#endif
	return pdFALSE;
}
static portBASE_TYPE speed_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	portBASE_TYPE p1len = 0;
	const char *p1 = FreeRTOS_CLIGetParameter(pcCommandString, 1, &p1len);
	int v = atoi(p1);
#if TRAINCTL_NEW == 0
	train_targetspeed(v);
#else
	train_set_target_speed(0, v);
#endif
	sprintf(pcWriteBuffer, "set target speed %d\r\n", v);
	return pdFALSE;
}
*/


static void param_set_pwm(struct param *p, int32_t v)
{
	set_pwm_freq(v);
}
static const param_t glob_params[] = {
		{ "pwmfreq", &cur_freqhz, 0, NULL, param_set_pwm, sizeof(int), 0, 60000,  50},

		{ NULL,     NULL,0,    NULL,NULL, 0, 0, 0,   0}
};



// 	void (*setter)(struct param *, int32_t);





