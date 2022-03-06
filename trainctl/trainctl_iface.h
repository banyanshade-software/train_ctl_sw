/*
 * trainctl_iface.h
 *
 *  Created on: Oct 25, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


/* trainctl_iface.h : this file should contain all exported symbols/definition
 *         that are needed to operate (either from RTOS tasks or from simulation program)
 *         the trainctl library
 */

#ifndef TRAINCTL_IFACE_H_
#define TRAINCTL_IFACE_H_

// ---------------------------------------------- traincontrol
#include "misc.h"

/*
#define NOTIF_NEW_ADC_1		0x00000001
#define NOTIF_NEW_ADC_2		0x00000002
#define NOTIF_TIM8   		0x00000004
#define NOTIF_TICKUI		0x00000008
#define NOTIF_STARTUP		0x80000000

*/


void train_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

int train_set_target_speed(int numtrain, int16_t target);
void calibrate_bemf(void);

void train_stop_all(void);

struct spd_notif {
    int16_t sv100;
    int16_t pid_target;
    int32_t bemf_centivolt;
};



extern uint32_t train_tick_last_dt;
extern uint32_t train_ntick;
// ---------------------------------------------- txrx

#define FRM_MAX_LEN 31

typedef struct {
	uint8_t t;
	uint8_t len;
	uint8_t frm[FRM_MAX_LEN];
} frame_msg_t;

#define RXFRAME_CHARS	   0xFF

#define TXFRAME_TYPE_RESP  1
#define TXFRAME_TYPE_NOTIF 2
#define TXFRAME_TYPE_DEBUG 3
#define TXFRAME_TYPE_STAT  4
#define TXFRAME_TYPE_OSCILO  5


void txrx_process_char(uint8_t c, uint8_t *respbuf, int *replen);
void txframe_send(frame_msg_t *m, int discardable);




// ---------------------------------------------- auto1
#define AUTO1_NOTIF_TICK		(0x00000001)
#define AUTO1_NOTIF_CMD_START	(0x00000002)
#define AUTO1_NOTIF_CMD_STOP	(0x00000004)

//extern void task_auto_tick_isr(void);
//extern void task_auto_tick(void);
extern void task_auto_start_auto(void);
extern void task_auto_stop_auto(void);

void auto1_run(uint32_t notif, uint32_t tick);

// ---------------------------------------------- canton

//#define BEMF_RAW 0



// ---------------------------------------------- param

typedef struct param param_t;

int param_get_value(const param_t *params, void *ptr, const char *name, int32_t *pvalue, int32_t *pdef, int32_t *pmin, int32_t *pmax);
int param_set_value(const param_t *params, void *ptr, const char *name, int32_t value);

// ---------------------------------------------- railconfig

void railconfig_setup_default(void);


// ---------------------------------------------- other

extern void set_pwm_freq(int freqhz, int crit);


#endif /* TRAINCTL_IFACE_H_ */
