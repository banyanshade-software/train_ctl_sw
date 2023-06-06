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


#if defined(STM32F4)
#include "stm32f4xx_hal.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"

#else
#error no board hal
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


#if SERVO_RUNS_ON_LED_TASK
#define POS_DIVISOR 10		// 1 if task run at 100hz, 10 if task run at 1000 etc..
#else
#define POS_DIVISOR 1
#endif


typedef struct {
	uint32_t curpos32;
	uint16_t target;
	uint16_t speed;
	uint8_t  sender;
	uint8_t  moving;
	uint8_t  powered:1;
    uint8_t  isdoor:1;
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
	default:
		itm_debug1(DBG_ERR, "led tim",0);
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
		var->curpos32 = POS_DIVISOR * conf->pos0;
		_servo_setpos(conf, var->target);
		var->moving = 4;
		var->sender = 0xFF; // no ack
	}
#ifndef TRAIN_SIMU
	if ((0)) {
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

static void servo_enter_runmode(_UNUSED_ runmode_t m)
{

}
static void process_servo_tick(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	//itm_debug1(DBG_SERVO, "servt", dt);
	for (int i=0; i < NUM_SERVOS; i++) {
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
#ifdef TRAIN_SIMU
                    /*
                     this is ugly but simu is normally only board 0, but in order to handle
                     current topology with servo on board 1, we need to do this for now
                     */
                    m.from = MA0_SERVO(1);
#else
					m.from = MA0_SERVO(oam_localBoardNum());
#endif
                    m.to = var->sender;
					m.subc = i;
                    if (!var->isdoor) {
                        m.cmd = CMD_SERVO_ACK;
                        m.v1u = var->curpos32/POS_DIVISOR;
                    } else {
                        m.cmd = CMD_SERVODOOR_ACK;
                        uint32_t exp = POS_DIVISOR * ((conf->direction) ? conf->max : conf->min);
                        m.v1u = (var->curpos32 == exp) ? 1 : 0;
                    }
					mqf_write_from_servo(&m);
				}
				if ((0)) {
						static int cnt=0;
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
			if (var->target*POS_DIVISOR != var->curpos32) {
				if (var->target*POS_DIVISOR > var->curpos32) {
					p = (var->curpos32 + var->speed);
					if (p > var->target*POS_DIVISOR) p = var->target*POS_DIVISOR;
				} else {
					p = (var->curpos32 - var->speed);
					if (p < var->target*POS_DIVISOR) p = var->target*POS_DIVISOR;
				}
				var->curpos32 = p;
				if ((1)) itm_debug3(DBG_SERVO, "mv", i,var->curpos32, var->target);
				_servo_setpos(conf, var->curpos32/POS_DIVISOR);
				var->moving = 4;
			}
		}
	}
}

static void servo_set(servo_var_t *var, const conf_servo_t *conf, uint16_t v, uint16_t spd, uint8_t sender)
{
    if (v > conf->max) v = conf->max;
    else if (v < conf->min) v = conf->min;
    var->target = v;
    var->moving = 0xFF;
    var->speed = spd ? spd : conf->spd;
    _servo_power(conf, 1);
    var->powered = 1;
    if ((sender == MA3_BROADCAST) || (sender == MA2_LOCAL_BCAST)) {
        var->sender = 0xFF;
    } else {
        var->sender = sender;
    }
}
static void process_servo_cmd(msg_64_t *m)
{
	switch(m->cmd) {
	default:
		itm_debug1(DBG_ERR|DBG_SERVO, "bad cmd", m->cmd);
		break;
	case CMD_SERVO_SET:
		itm_debug3(DBG_SERVO, "set", m->subc, m->v1u, m->v2u);
		if (m->subc > NUM_SERVOS) break;
		{
			USE_SERVO(m->subc);
			servo_set(var, conf, m->v1u, m->v2u, m->from);
            var->isdoor = 0;
		}
		break;
	case CMD_SERVODOOR_SET:
		itm_debug2(DBG_SERVO, "setdoor", m->subc, m->v1u);
		if (m->subc > NUM_SERVOS) break;
		{
			USE_SERVO(m->subc);
			uint16_t p;
			if (conf->direction) {
				p = m->v1u ? conf->max : conf->min;
			} else {
				p = m->v1u ? conf->min : conf->max;
			}
			servo_set(var, conf, p, 0, m->from);
            var->isdoor = 1;
		}
		break;

	}
}

