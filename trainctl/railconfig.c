/*
 * railconfig.c
 *
 *  Created on: Oct 8, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "misc.h"
#include "railconfig.h"
#include "canton.h"
#include "train.h"
#include "turnout.h"
#include "param.h"


#ifndef TRAIN_SIMU
//#include "main.h"
#else

#define VOLT_SEL0_GPIO_Port NULL
#define VOLT_SEL0_Pin 0
#define VOLT_SEL1_Pin 0
#define VOLT_SEL2_Pin 0
#define VOLT_SEL3_Pin 0

#define TIM1 NULL
#define TIM_CHANNEL_1 1
#define TIM_CHANNEL_2 2
#endif


static  canton_config_t Cantons[NUM_CANTONS] = {
		{CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
				VOLT_SEL0_GPIO_Port,
				VOLT_SEL0_Pin, VOLT_SEL1_Pin, VOLT_SEL2_Pin, VOLT_SEL3_Pin,
				1, TIM_CHANNEL_1, TIM_CHANNEL_2,  // TIM_HandleTypeDef
				1, /*notif BEMF */
		},

		{CANTON_TYPE_DUMMY,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345},// volts[16]
				NULL, 0, 0, 0, 0,
				0, 0, 0,
				0, /* notif BEMF */
		},
};


static  train_config_t Trains[NUM_TRAINS] = {
		{
				{ // pidctl_config_t
						50, 30, 350,  // kP, kI, kD
                    /*
                     * 50, 30, -50,
                     */
				},
				{ // inertia_config_t
						150, 100		// dec, acc
				},
				vpolicy_normal,  //vpolicy_normal, // vpolicy_pure_volt, //vpolicy_pure_pwm,
				0, // enable_inertia
				1, // enable_pid
				1, // notify_speed
				1, // notify_pose
				1, // bemfIIR;
				1, // fix_bemf;
				1,  //	uint8_t en_spd2pow;
				30, //	uint8_t min_power;
		}
};


static const turnout_config_t Turnouts[NUM_TURNOUTS] = {
#ifndef TRAIN_SIMU
        {TURN1A_GPIO_Port, TURN1A_Pin, TURN1B_Pin,
#else
            {NULL, 1, 2,
#endif
            }
};


static canton_vars_t  CantonsVars[NUM_CANTONS];
static train_vars_t   TrainsVars[NUM_TRAINS];
static turnout_vars_t TurnoutVars[NUM_TURNOUTS];

static int setup_done = 0;

const canton_config_t *get_canton_cnf(int idx)
{
	if (!setup_done) return config_error(-1, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &Cantons[idx];
}

canton_vars_t *get_canton_vars(int idx)
{
	if (!setup_done) return config_error(-1, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &CantonsVars[idx];
}

const train_config_t *get_train_cnf(int idx)
{
	if (!setup_done) return config_error(-1, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TRAINS)) return NULL;
	return &Trains[idx];
}

train_vars_t *get_train_vars(int idx)
{
	if (!setup_done) return config_error(-1, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TRAINS)) return NULL;
	return &TrainsVars[idx];
}


const turnout_config_t  *get_turnout_cnf(int idx)
{
	if (!setup_done) return config_error(-1, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TURNOUTS)) return NULL;
	return &Turnouts[idx];
}
turnout_vars_t  *get_turnout_vars(int idx)
{
	if (!setup_done) return config_error(-1, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TURNOUTS)) return NULL;
	return &TurnoutVars[idx];
}


int canton_idx(canton_vars_t *v)
{
	return (int)(v-&CantonsVars[0]);
}
int train_idx(train_vars_t *v)
{
	return (int)(v-&TrainsVars[0]);
}

int turnout_idx(turnout_vars_t *v)
{
	return (int)(v-&TurnoutVars[0]);
}

void railconfig_setup_default(void)
{
	for (int i=0; i<NUM_CANTONS; i++) {
		canton_reset(&Cantons[i], &CantonsVars[i]);
	}
	for (int i=0; i<NUM_TRAINS; i++) {
		train_reset(&Trains[i], &TrainsVars[i]);
	}
    canton_take(&Cantons[0], &CantonsVars[0], canton_occupied_loco,  0);
    TrainsVars[0].current_canton = 0;
    TrainsVars[0].current_canton_dir = 1;
    canton_take(&Cantons[1], &CantonsVars[1], canton_occupied_loco,  0);
    TrainsVars[0].next_canton = 1;
    TrainsVars[0].next_canton_dir = 1;
	setup_done = 1;
	for (int i=0; i<NUM_TURNOUTS; i++) {
		turnout_reset(i);
	}
}



/* ------------------------------------------------------------------------------- */

const param_t train_params[] = {
		{ "kp",		 NULL, offsetof(train_config_t, pidcnf.kP), 		NULL,NULL, sizeof(int32_t), -3000, 3000,  150},
		{ "ki",		 NULL, offsetof(train_config_t, pidcnf.kI), 		NULL,NULL, sizeof(int32_t), -3000, 3000,  50},
		{ "kd",   	 NULL, offsetof(train_config_t, pidcnf.kD),			NULL,NULL, sizeof(int32_t), -3000, 3000, 550},
		{ "dec",   	 NULL, offsetof(train_config_t, inertiacnf.dec),    NULL,NULL, sizeof(int16_t), 0, 1000,   300},
		{ "acc",  	 NULL, offsetof(train_config_t, inertiacnf.acc),   	NULL,NULL, sizeof(int16_t), 0, 1000,   200},

		{"en_inertia", NULL, offsetof(train_config_t,enable_inertia),	NULL,NULL, sizeof(uint8_t), 0, 1, 0},
		{"en_pid",     NULL, offsetof(train_config_t,enable_pid),		NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"notify_spd", NULL, offsetof(train_config_t,notify_speed),     NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"notify_pose",NULL, offsetof(train_config_t,notify_pose),      NULL,NULL, sizeof(uint8_t), 0, 1, 0},
		{"bemfIIR",    NULL, offsetof(train_config_t,bemfIIR),   	    NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"fix_bemf",   NULL, offsetof(train_config_t,fix_bemf),   	    NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"volt_policy",NULL, offsetof(train_config_t,volt_policy),      NULL,NULL, sizeof(uint8_t), 0, 1, 0},
		{"en_spd2pow", NULL, offsetof(train_config_t,en_spd2pow),       NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"min_power",  NULL, offsetof(train_config_t,min_power),        NULL,NULL, sizeof(uint8_t), 0, 80, 40},

		{ NULL,     NULL,0,    NULL,NULL, 0, 0, 0,   0}
};
