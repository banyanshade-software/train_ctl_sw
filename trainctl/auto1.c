/*
 * auto1.c
 *
 *  Created on: Oct 16, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/*
 * auto1.c : a temporary very simple automatisation, for a single block railway
 *           (good for now to take videos of the train)
 *           this is really intended to be removed and replaced by something more useful
 */


#include "misc.h"
#include "auto1.h"
#include "train.h"
#include "low/turnout.h"
#include "traincontrol.h"
#include "railconfig.h"

typedef struct {
	int16_t speed;
	int16_t wait;
	int32_t poselim;
	int8_t  turnoutcmd;
	int8_t  jump;
} auto_step_t;

static const auto_step_t steps[] = {
		/* 0 */
		/*         spd wait poselim turnout jmp*/
		/*0*/	{  0,  500,  0,       -1,      -1 },
		/*1*/	{  0,  500,  0,       -1,      -1 },
		/*2*/	{  20,  0,  10000,    0,      -1 },
		/*3*/	{  10,  0,  22000,    1,      -1 },
		/*4*/	{   0, 1000, 0, 		  1,      -1 },
		/*5*/   {  -30, 0,  9000,     0,	 -1 },
		/*6*/   {  -10, 0,  7000,     0,	 -1 },
		/*7 */  {   20, 0,  10000,    0,     -1 },
		/*8 */  {   35, 0,  16000,    0,     -1 },
		/*9 */  {  -10, 0, 10000,    -1,     -1 },
		/*10*/  {  -15, 0,  4000,    -1,      -1 },
		/* 11*/	{    0,20*1000,  0,       0,       0}
};

typedef struct {
	int step;
	uint32_t pose_offset;
	uint32_t tickstop;
} auto1_vars_t;

static auto1_vars_t vars = {-1, 0, 0};


void auto1_reset(void)
{
	debug_info('T', 0, "A1/RST", 0,0,0);
    vars.step = -1;
}


static void bh(void) {
    //printf("hop");
}

#define GO_STEP(_s) do { 	\
    if (vars.step==11) {     \
bh();                       \
    }                       \
	vars.step = _s; 		\
	goto new_step;			\
} while (0)

#define NEXT_STEP() do { 				\
	if (steps[vars.step].jump >=0) {	\
		GO_STEP(steps[vars.step].jump);	\
	} else {							\
		GO_STEP(vars.step+1);			\
	} 									\
} while (0)

void auto1_run(uint32_t notif, uint32_t tick)
{
	const train_config_t *t0cnf = get_train_cnf(0);
	train_vars_t *t0vars        = get_train_vars(0);
	const auto_step_t *stp;
	int32_t pose = t0vars->position_estimate - vars.pose_offset;

	if (-1 == vars.step) {
		if ((notif & AUTO1_NOTIF_CMD_START) && t0cnf && t0vars) {
			vars.pose_offset = t0vars->position_estimate;
			debug_info('T', 0, "A1/START", vars.step,vars.pose_offset,0);
			GO_STEP(0);
		}
	}
	if (notif & AUTO1_NOTIF_CMD_STOP) {
		debug_info('T', 0, "A1/STOP", vars.step,0,0);
		if (-1 == vars.step) return;
		train_set_target_speed(0, 0);
		vars.step = -1;
		return;
	}
	if (-1 == vars.step) return;

	if (vars.tickstop>0) {
		if (tick >= vars.tickstop) NEXT_STEP();
	}
	if (steps[vars.step].speed > 0) {
		if (pose  > steps[vars.step].poselim) NEXT_STEP();
	} else {
		/*if (-15==steps[vars.step].speed) {
			debug_info('T', 0, "A1/10", vars.step, pose, steps[vars.step].poselim);
		}*/
		if (pose <= steps[vars.step].poselim) NEXT_STEP();
	}
	return;

new_step:
	stp = &steps[vars.step];
	debug_info('T', 0, "A1/CHG", vars.step, stp->speed, pose);
	if (stp->wait) {
		vars.tickstop = tick + stp->wait;
	} else {
		vars.tickstop = 0;
	}
	if (stp->turnoutcmd) {
		turnout_cmd(0, stp->turnoutcmd);
	}
	train_set_target_speed(0, stp->speed);
	if ((1)) {
		int16_t v[2];
		v[0] = vars.step;
		v[1] = stp->speed;
		train_notif(0, 'A', (void *)v, sizeof(int16_t)*2);
	}
}


