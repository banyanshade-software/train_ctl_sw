/*
 * servo.c
 *
 *  Created on: Nov 2, 2022
 *      Author: danielbraun
 */



#include <stdint.h>
#include <stddef.h>
#include <memory.h>


#include "servo.h"
#include "../config/conf_servo.h"

#include "../misc.h"
#include "../msg/trainmsg.h"
#include "../msg/tasklet.h"

#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#else
#include "train_simu.h"
#endif


#ifndef BOARD_HAS_SERVOS
#error BOARD_HAS_SERVOS not defined, remove this file from build
#endif


#if (NUM_SERVOS == 0)
#error turnouts code included but NUM_SERVOS iz zero
#endif


static void servo_init(void);
static void servo_enter_runmode(runmode_t m);
static void process_servo_tick(uint32_t tick, uint32_t dt);
static void process_servo_cmd(msg_64_t *m);


static const tasklet_def_t servo_tdef = {
		.init 				= servo_init,
		.poll_divisor		= NULL,
		.emergency_stop 	= servo_init,
		.enter_runmode		= servo_enter_runmode,
		.pre_tick_handler	= NULL,
		.default_msg_handler = process_servo_cmd,
		.default_tick_handler = process_servo_tick,
		.msg_handler_for	= NULL,
		.tick_handler_for 	= NULL
};
tasklet_t servo_tasklet = { .def = &servo_tdef, .init_done = 0, .queue=&to_servo};



typedef struct {
	uint16_t target;
	uint16_t curpos;
	uint16_t speed;
	uint8_t  sender;
	uint8_t  moving;
	uint8_t  powered:1;
} servo_var_t;

static servo_var_t servo_vars[NUM_SERVOS] = {0};


#define USE_SERVO(_idx) \
		const conf_servo_t *conf = conf_servo_get(_idx); \
		servo_var_t       *var = &servo_vars[_idx]


// ----------------------------------------------------------
#ifndef TRAIN_SIMU
static TIM_HandleTypeDef *ServoTimerHandles[5] = {0};


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
#endif // TRAIN_SIMU


static void _servo_setpos(const conf_servo_t *conf, uint16_t pos)
{
#ifndef TRAIN_SIMU
	//  set pwm timer
	TIM_HandleTypeDef *tim = NULL;
	if (conf->pwm_timer_num > 4) return;
	tim = ServoTimerHandles[conf->pwm_timer_num];
	if (!tim) return;

	uint32_t t = ((pos+2000) * 64) / 100;
	switch (conf->pwm_timer_ch) {
	case TIM_CHANNEL_1:
		tim->Instance->CCR1 = t;
		break;
	case TIM_CHANNEL_2:
		tim->Instance->CCR2 = t;
		break;
	case TIM_CHANNEL_3:
		tim->Instance->CCR3 = t;
		break;
	case TIM_CHANNEL_4:
		tim->Instance->CCR4 = t;
		break;
	}
#endif
}
static void _servo_power(const conf_servo_t *conf, uint8_t pow)
{
	if (conf->pin_power<0) return;
#ifndef TRAIN_SIMU
	HAL_GPIO_WritePin(conf->port_power, conf->pin_power,
			pow ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

#ifndef TRAIN_SIMU
static uint8_t timstarted = 0;
#endif

static void _timer_init(unsigned int timnum, int ch)
{
#ifndef TRAIN_SIMU
	TIM_HandleTypeDef *tim = NULL;
	if (timnum > 4) return;
	tim = ServoTimerHandles[timnum];

	if (!tim) return;
	if ((timstarted & (1<<timnum)) == 0) {
		HAL_TIM_Base_Start(tim);
		timstarted |= (1<<timnum);
	}
	HAL_TIM_PWM_Start(tim, ch);
#endif
}
// ----------------------------------------------------------

static void servo_init(void)
{
#ifndef TRAIN_SIMU
    ServoTimerHandles[1]=&htim1;
	ServoTimerHandles[2]=&htim2;
	ServoTimerHandles[3]=&htim3;
	ServoTimerHandles[4]=&htim4;
#endif
    
	memset(&servo_vars, 0, sizeof(servo_vars));
	for (int i=0; i<NUM_SERVOS; i++) {
		USE_SERVO(i);
		if (conf->pwm_timer_num<0) continue;
		_timer_init(conf->pwm_timer_num, conf->pwm_timer_ch);
		_servo_power(conf, 1);
		var->powered = 1;
		var->target = conf->pos0;
		var->curpos = conf->pos0;
		_servo_setpos(conf, var->target);
		var->moving = 4;
		var->sender = 0xFF; // no ack
	}
#ifndef TRAIN_SIMU
	if ((1)) {
		msg_64_t m = {0};
		m.cmd = CMD_SERVO_SET;
		m.from = MA3_BROADCAST;
		m.to = MA0_SERVO(oam_localBoardNum());
		m.subc = 0;
		m.v1u = 9001; //9000;
		m.v2u = 4;
		mqf_write_from_nowhere(&m);
	}
#endif
}

static void servo_enter_runmode(runmode_t m)
{

}
static void process_servo_tick(uint32_t tick, uint32_t dt)
{
	//itm_debug1(DBG_SERVO, "servt", dt);
	for (int i=0; i<NUM_SERVOS; i++) {
		USE_SERVO(i);
		if (var->moving) {
			//if (i) {
			//	itm_debug2(DBG_SERVO, "tick", i, var->moving);
			//}
			var->moving--;
			if (!var->moving) {
				// position reach and timer
				_servo_power(conf, 0);
				var->powered = 0;
				itm_debug2(DBG_SERVO, "stp", i, var->sender);
				if (var->sender != 0xFF) {
					msg_64_t m = {0};
					m.from = MA0_SERVO(oam_localBoardNum());
                    m.to = var->sender;
					m.subc = i;
					m.cmd = CMD_SERVO_ACK;
					m.v1u = var->curpos;
					mqf_write_from_servo(&m);
				}
				if ((1)) {
						static cnt=0;
						msg_64_t m = {0};
						m.cmd = CMD_SERVO_SET;
						m.from = MA3_BROADCAST;
						m.to = MA0_SERVO(oam_localBoardNum());
						m.subc = 0;
						m.v1u = (cnt%2) ? 0 : 10000;
						m.v2u = 50;
						mqf_write_from_nowhere(&m);
						cnt++;
					}
				continue;
			}
			int32_t p;
			if (var->target != var->curpos) {
				if (var->target - var->curpos > 0) {
					p = var->curpos + var->speed;
					if (p > var->target) p = var->target;
				} else {
					p = var->curpos - var->speed;
					if (p < var->target) p = var->target;
				}
				var->curpos = (uint16_t) p;
				itm_debug3(DBG_SERVO, "mv", i,var->curpos, var->target);
				_servo_setpos(conf, var->curpos);
				var->moving = 4;
			}
		}
	}
}


static void process_servo_cmd(msg_64_t *m)
{
	switch(m->cmd) {
	case CMD_SERVO_SET:
		itm_debug3(DBG_SERVO, "set", m->subc, m->v1u, m->v2u);
		if (m->subc > NUM_SERVOS) break;
		USE_SERVO(m->subc);
		uint16_t v = m->v1u;
		if (v > conf->max) v = conf->max;
		else if (v < conf->min) v = conf->min;
		var->target =v;
		var->moving = 0xFF;
		var->speed = m->v2u ? m->v2u : conf->spd;
		_servo_power(conf, 1);
		var->powered = 1;
		if ((m->from == MA3_BROADCAST) || (m->from != MA2_LOCAL_BCAST)) {
			var->sender = 0xFF;
		} else {
			var->sender = m->from;
		}
	}
}

