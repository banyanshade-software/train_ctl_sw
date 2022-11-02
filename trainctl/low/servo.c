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

static void _servo_setpos(const conf_servo_t *conf, uint16_t pos)
{
	// XXX set pwm timer
}
static void _servo_power(const conf_servo_t *conf, uint8_t pow)
{
	// TODO
}
// ----------------------------------------------------------

static void servo_init(void)
{
	memset(&servo_vars, 0, sizeof(servo_vars));
	for (int i=0; i<NUM_SERVOS; i++) {
		USE_SERVO(i);
		if (conf->pwm_timer_num<0) continue;
		_servo_power(conf, 1);
		var->powered = 1;
		var->target = conf->pos0;
		var->curpos = conf->pos0;
		_servo_setpos(conf, var->target);
		var->moving = 0xFF;
	}
}
static void servo_enter_runmode(runmode_t m)
{

}
static void process_servo_tick(uint32_t tick, uint32_t dt)
{
	for (int i=0; i<NUM_SERVOS; i++) {
		USE_SERVO(i);
		if (var->moving) {
			var->moving--;
			if (!var->moving) {
				// position reach and timer
				_servo_power(conf, 0);
				var->powered = 0;
				if (var->sender) {
					msg_64_t m = {0};
					m.from = MA0_SERVO(oam_localBoardNum());
					m.subc = i;
					m.cmd = CMD_SERVO_ACK;
					m.v1u = var->curpos;
					mqf_write_from_servo(&m);
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
				_servo_setpos(conf, var->curpos);
				var->moving = 0xFF;
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
		var->target = m->v1u;
		var->moving = 0xFF;
		var->speed = m->v2u ? m->v2u : conf->spd;
		_servo_power(conf, 1);
		var->powered = 1;
		var->sender = m->from;
	}
}

