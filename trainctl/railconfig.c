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


/* railconfig.h : this is the main storage for configuration and runtime info
 *                related to train, blocks, turnout, etc..
 *                We basically maitains arrays for all trains, all blocks, etc..
 * 				  Configuration upload/download is planned here, but not implemented
 */


/* 20201204 good setup for lowspeed 8875 :
 200Hz
 spd_to_pow disabled
 PID 900/500/3000
 pre-iir
 */
#include "misc.h"
#include "railconfig.h"
#include "low/canton_config.h"
#include "train.h"
#include "low/turnout_config.h"
#include "param.h"


#ifndef TRAIN_SIMU
#include "main.h"

#if 0
#ifndef VOLT_0_SEL0_Pin
#define VOLT_0_SEL0_Pin VOLT_SEL0_Pin
#endif
#ifndef VOLT_0_SEL1_Pin
#define VOLT_0_SEL1_Pin VOLT_SEL1_Pin
#endif
#ifndef VOLT_0_SEL2_Pin
#define VOLT_0_SEL2_Pin VOLT_SEL2_Pin
#endif
#ifndef VOLT_0_SEL3_Pin
#define VOLT_0_SEL3_Pin VOLT_SEL3_Pin
#endif

#ifndef VOLT_0_SEL0_GPIO_Port
#define VOLT_0_SEL0_GPIO_Port VOLT_SEL0_GPIO_Port
#endif

#ifndef VOLT_0_SEL1_GPIO_Port
#define VOLT_0_SEL1_GPIO_Port VOLT_SEL1_GPIO_Port
#endif


#ifndef VOLT_0_SEL2_GPIO_Port
#define VOLT_0_SEL2_GPIO_Port VOLT_SEL2_GPIO_Port
#endif


#ifndef VOLT_0_SEL3_GPIO_Port
#define VOLT_0_SEL3_GPIO_Port VOLT_SEL3_GPIO_Port
#endif

#endif


#else

//simu
#define VOLT_0_SEL0_GPIO_Port NULL
#define VOLT_0_SEL1_GPIO_Port NULL
#define VOLT_0_SEL2_GPIO_Port NULL
#define VOLT_0_SEL3_GPIO_Port NULL
#define VOLT_0_SEL0_Pin 0
#define VOLT_0_SEL1_Pin 0
#define VOLT_0_SEL2_Pin 0
#define VOLT_0_SEL3_Pin 0

#define VOLT_1_SEL0_GPIO_Port NULL
#define VOLT_1_SEL1_GPIO_Port NULL
#define VOLT_1_SEL2_GPIO_Port NULL
#define VOLT_1_SEL3_GPIO_Port NULL
#define VOLT_1_SEL0_Pin 0
#define VOLT_1_SEL1_Pin 0
#define VOLT_1_SEL2_Pin 0
#define VOLT_1_SEL3_Pin 0


#define VOLT_2_SEL0_GPIO_Port NULL
#define VOLT_2_SEL1_GPIO_Port NULL
#define VOLT_2_SEL2_GPIO_Port NULL
#define VOLT_2_SEL3_GPIO_Port NULL
#define VOLT_2_SEL0_Pin 0
#define VOLT_2_SEL1_Pin 0
#define VOLT_2_SEL2_Pin 0
#define VOLT_2_SEL3_Pin 0


#define VOLT_3_SEL0_GPIO_Port NULL
#define VOLT_3_SEL1_GPIO_Port NULL
#define VOLT_3_SEL2_GPIO_Port NULL
#define VOLT_3_SEL3_GPIO_Port NULL
#define VOLT_3_SEL0_Pin 0
#define VOLT_3_SEL1_Pin 0
#define VOLT_3_SEL2_Pin 0
#define VOLT_3_SEL3_Pin 0

#define VOLT_4_SEL0_GPIO_Port NULL
#define VOLT_4_SEL1_GPIO_Port NULL
#define VOLT_4_SEL2_GPIO_Port NULL
#define VOLT_4_SEL3_GPIO_Port NULL
#define VOLT_4_SEL0_Pin 0
#define VOLT_4_SEL1_Pin 0
#define VOLT_4_SEL2_Pin 0
#define VOLT_4_SEL3_Pin 0


#define TIM1 NULL
#define TIM_CHANNEL_1 1
#define TIM_CHANNEL_2 2
#define TIM_CHANNEL_3 3
#define TIM_CHANNEL_4 4
#endif




static  canton_config_t Cantons[NUM_CANTONS] = {
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
				{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]
                //{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
                //{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_0_SEL0_GPIO_Port,VOLT_0_SEL1_GPIO_Port, VOLT_0_SEL2_GPIO_Port, //0/*VOLT_0_SEL3_GPIO_Port*/,
				VOLT_0_SEL0_Pin, VOLT_0_SEL1_Pin, VOLT_0_SEL2_Pin,// 0/*VOLT_0_SEL3_Pin*/,
				1, TIM_CHANNEL_1, TIM_CHANNEL_2,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				1, /* reverse BEMF*/
		},
#if NUM_LOCAL_CANTONS_SW == 8
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]				//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_1_SEL0_GPIO_Port,VOLT_1_SEL1_GPIO_Port, VOLT_1_SEL2_GPIO_Port,//0/*VOLT_1_SEL3_GPIO_Port*/,
				VOLT_1_SEL0_Pin, VOLT_1_SEL1_Pin, VOLT_1_SEL2_Pin, //0/*VOLT_1_SEL3_Pin*/,
				1, TIM_CHANNEL_3, TIM_CHANNEL_4,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				0, /* reverse BEMF*/
		},
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]				//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_2_SEL0_GPIO_Port,VOLT_2_SEL1_GPIO_Port, VOLT_2_SEL2_GPIO_Port,//0/*VOLT_2_SEL3_GPIO_Port*/,
				VOLT_2_SEL0_Pin, VOLT_2_SEL1_Pin, VOLT_2_SEL2_Pin, // 0/*VOLT_2_SEL3_Pin*/,
				2, TIM_CHANNEL_3, TIM_CHANNEL_4,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				0, /* reverse BEMF*/
		},
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]				//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_3_SEL0_GPIO_Port, VOLT_3_SEL1_GPIO_Port, VOLT_3_SEL2_GPIO_Port, //0/*VOLT_3_SEL3_GPIO_Port*/,
				VOLT_3_SEL0_Pin, VOLT_3_SEL1_Pin, VOLT_3_SEL2_Pin,// 0/* VOLT_3_SEL3_Pin*/,
				3, TIM_CHANNEL_1, TIM_CHANNEL_2,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				0, /* reverse BEMF*/
		},
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
				{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]						//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_4_SEL0_GPIO_Port, VOLT_4_SEL1_GPIO_Port, VOLT_4_SEL2_GPIO_Port, //0/*VOLT_4_SEL3_GPIO_Port*/,
				VOLT_4_SEL0_Pin, VOLT_4_SEL1_Pin, VOLT_4_SEL2_Pin, //0/* VOLT_4_SEL3_Pin*/,
				3, TIM_CHANNEL_3, TIM_CHANNEL_4,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				0, /* reverse BEMF*/
		},
#ifdef VOLT_5_SEL0_GPIO_Port
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
				{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]						//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_5_SEL0_GPIO_Port, VOLT_5_SEL1_GPIO_Port, VOLT_5_SEL2_GPIO_Port, //0/*VOLT_4_SEL3_GPIO_Port*/,
				VOLT_5_SEL0_Pin, VOLT_5_SEL1_Pin, VOLT_5_SEL2_Pin, //0/* VOLT_4_SEL3_Pin*/,
				4, TIM_CHANNEL_1, TIM_CHANNEL_1,  // TIM_HandleTypeDef 4==TIM12
				0, /*notif BEMF */
				0, /* reverse BEMF*/
		},
#else
		{
						{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]						//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
						NULL, NULL, NULL,
						0, 0, 0,
						0, 0, 0,
						0, 0
				},
#endif
		{
				{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]						//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				NULL, NULL, NULL,
				0, 0, 0,
				0, 0, 0,
				0, 0
		},
		{
				{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]						//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				NULL, NULL, NULL,
				0, 0, 0,
				0, 0, 0,
				0, 0
		}

#else
#error holala
#endif

#ifdef ADD_DUMMY_CANTON
#error obsolete
		{//CANTON_TYPE_DUMMY,
            //  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]            //{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
            //{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4
				NULL,NULL,NULL /*,NULL*/, 0, 0, 0, //0,
				0, 0, 0,
				0, /* notif BEMF */
		},
#endif
};

#if 0
static  block_canton_config_t BlockCantons[NUM_CANTONS] = {
		/*  uint8_t left_a;
			uint8_t left_b;
				uint8_t left_turnout;
			uint8_t right_a;
			uint8_t right_b;
				uint8_t right_turnout;
			uint8_t len; */
		{{0xFF, 0xFF, 0xFF},    {0x02, 0xFF, 0xFF},   50},	// 0
		{{0xFF, 0xFF, 0xFF},    {0x02, 0xFF, 0xFF},   50},  // 1
		{{0x00, 0x01, 0   },    {0xFF, 0xFF, 0xFF},   50},  // 2

		{{0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF},  50},
		{{0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF},  50},

};
#endif

#define DEFAULT_TRAIN_CFG(_EN)  { \
						{ /* pidctl_config_t*/ \
								600, 500, 750,  /* kP, kI, kD */ \
						}, \
						{ /* inertia_config_t */ \
								350, 200		/* dec, acc */ \
						}, \
						vpolicy_normal,  /*vpolicy_normal, vpolicy_pure_volt, vpolicy_pure_pwm,*/ \
						0, /* enable_inertia */		\
						_EN, /* enabled */			\
						1, /* enable_pid */			\
						0, /* notify_speed */		\
						0, /* notify_pose */		\
						1, /* bemfIIR; */			\
						0, /* postIIR */			\
						0, /* fix_bemf; */			\
						0,  /*	uint8_t en_spd2pow; */\
						20, /*	uint8_t min_power; */ \
				}

static  train_config_t Trains[NUM_TRAINS] = {
		DEFAULT_TRAIN_CFG(1), DEFAULT_TRAIN_CFG(0), DEFAULT_TRAIN_CFG(0), DEFAULT_TRAIN_CFG(0),
		DEFAULT_TRAIN_CFG(0), DEFAULT_TRAIN_CFG(0), DEFAULT_TRAIN_CFG(0), DEFAULT_TRAIN_CFG(0),
};


static const turnout_config_t Turnouts[NUM_TURNOUTS] = {
#ifndef TRAIN_SIMU
        {TURN1A_GPIO_Port, TURN1A_Pin, TURN1B_Pin,
#else
            {NULL, 1, 2,
#endif
            }
};


        //static canton_vars_t  CantonsVars[NUM_CANTONS];
        //static block_canton_vars_t  BlockCantonsVars[NUM_CANTONS];
        //static train_vars_t   TrainsVars[NUM_TRAINS];
        //static turnout_vars_t TurnoutVars[NUM_TURNOUTS];

static int setup_done = 1;

const canton_config_t *get_canton_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &Cantons[idx];
}
/*
canton_vars_t *get_canton_vars(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &CantonsVars[idx];
}
*/
/*
const block_canton_config_t *get_block_canton_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &BlockCantons[idx];
}

block_canton_vars_t *get_block_canton_vars(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &BlockCantonsVars[idx];
}
*/
const train_config_t *get_train_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TRAINS)) return NULL;
	return &Trains[idx];
}

/*
train_vars_t *get_train_vars(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TRAINS)) return NULL;
	return &TrainsVars[idx];
}

*/
const turnout_config_t  *get_turnout_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TURNOUTS)) return NULL;
	return &Turnouts[idx];
}
/*
turnout_vars_t  *get_turnout_vars(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TURNOUTS)) return NULL;
	return &TurnoutVars[idx];
}
*/

/*
int canton_idx(canton_vars_t *v)
{
	return (int)(v-&CantonsVars[0]);
}

int block_canton_idx(block_canton_vars_t *v)
{
	return (int)(v-&BlockCantonsVars[0]);
}
int train_idx(train_vars_t *v)
{
	return (int)(v-&TrainsVars[0]);
}

int turnout_idx(turnout_vars_t *v)
{
	return (int)(v-&TurnoutVars[0]);
}
*/

/*
void railconfig_setup_default(void)
{
    setup_done = 1;
	for (int i=0; i<NUM_CANTONS; i++) {
		canton_reset(&Cantons[i], &CantonsVars[i]);
		block_canton_reset(&BlockCantons[i], &BlockCantonsVars[i]);
	}
	for (int i=0; i<NUM_TRAINS; i++) {
		train_reset(&Trains[i], &TrainsVars[i]);
	}
    canton_set_train(0,  0);
    //canton_take(0, canton_occupied_loco,  0);
    TrainsVars[0].current_canton = 0;
    TrainsVars[0].current_canton_dir = 1;
    canton_set_train(2,  0);
    //canton_take(1, canton_occupied_loco,  0);
    TrainsVars[0].next_canton = 2;
    TrainsVars[0].next_canton_dir = 0;
    //TrainsVars[0].prev_canton = 0xFF;
    //TrainsVars[0].prev_canton_dir = 0;
    for (int i=0; i<NUM_TURNOUTS; i++) {
		turnout_reset(i);
	}
}
*/


