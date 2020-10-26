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

#ifndef TRAINCTL_IFACE_H_
#define TRAINCTL_IFACE_H_

// ---------------------------------------------- traincontrol



#define NOTIF_NEW_ADC_1		0x00000001
#define NOTIF_NEW_ADC_2		0x00000002
#define NOTIF_STARTUP		0x80000000



void train_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

int train_set_target_speed(int numtrain, int16_t target);
void calibrate_bemf(void);

void train_stop_all(void);

struct spd_notif {
    int16_t sv100;
    int16_t pid_target;
    int32_t bemf_centivolt;
};

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

#define BEMF_RAW 0


// notif_flags
typedef struct {
	uint16_t voffA;
	uint16_t voffB;
	//uint16_t intOff; // XXX
	uint16_t vonA;
	uint16_t vonB;
	//uint16_t intOn; // XXX

} adc_buffer_t;

extern volatile adc_buffer_t train_adc_buffer[];

// ---------------------------------------------- param

typedef struct param param_t;

int param_get_value(const param_t *params, void *ptr, const char *name, int32_t *pvalue, int32_t *pdef, int32_t *pmin, int32_t *pmax);
int param_set_value(const param_t *params, void *ptr, const char *name, int32_t value);

// ---------------------------------------------- railconfig

void railconfig_setup_default(void);


// ---------------------------------------------- other

extern int cur_freqhz;
extern void set_pwm_freq(int freqhz);


#endif /* TRAINCTL_IFACE_H_ */
