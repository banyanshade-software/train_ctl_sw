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
    // C0
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
	// C1
        {//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]				//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_1_SEL0_GPIO_Port,VOLT_1_SEL1_GPIO_Port, VOLT_1_SEL2_GPIO_Port,//0/*VOLT_1_SEL3_GPIO_Port*/,
				VOLT_1_SEL0_Pin, VOLT_1_SEL1_Pin, VOLT_1_SEL2_Pin, //0/*VOLT_1_SEL3_Pin*/,
				1, TIM_CHANNEL_3, TIM_CHANNEL_4,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				1, /* reverse BEMF*/
		},
    // C2
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]				//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_2_SEL0_GPIO_Port,VOLT_2_SEL1_GPIO_Port, VOLT_2_SEL2_GPIO_Port,//0/*VOLT_2_SEL3_GPIO_Port*/,
				VOLT_2_SEL0_Pin, VOLT_2_SEL1_Pin, VOLT_2_SEL2_Pin, // 0/*VOLT_2_SEL3_Pin*/,
				2, TIM_CHANNEL_3, TIM_CHANNEL_4,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				1, /* reverse BEMF*/
		},
    // C3
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
								{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]				//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_3_SEL0_GPIO_Port, VOLT_3_SEL1_GPIO_Port, VOLT_3_SEL2_GPIO_Port, //0/*VOLT_3_SEL3_GPIO_Port*/,
				VOLT_3_SEL0_Pin, VOLT_3_SEL1_Pin, VOLT_3_SEL2_Pin,// 0/* VOLT_3_SEL3_Pin*/,
				3, TIM_CHANNEL_1, TIM_CHANNEL_2,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				1, /* reverse BEMF*/
		},
    // C4
		{//CANTON_TYPE_PROTO1,
				//  0    1    2    3    4    5    6    7    8    9    10   11  12    13   14   15
				//{ 1000, 874, 770, 699, 621, 578, 538, 507, 451, 432, 413, 398, 379, 367, 355, 345}, // volts[16]
				{ 1000, 770, 621,  538, 451, 413, 379, 355}, // volts[8]						//{ 1000, 0,    0, 0, 0, 621, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V2
				//{ 1000, 874,  0, 0, 0, 621, 538, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // volts[16]  V4

				VOLT_4_SEL0_GPIO_Port, VOLT_4_SEL1_GPIO_Port, VOLT_4_SEL2_GPIO_Port, //0/*VOLT_4_SEL3_GPIO_Port*/,
				VOLT_4_SEL0_Pin, VOLT_4_SEL1_Pin, VOLT_4_SEL2_Pin, //0/* VOLT_4_SEL3_Pin*/,
				3, TIM_CHANNEL_3, TIM_CHANNEL_4,  // TIM_HandleTypeDef
				0, /*notif BEMF */
				1, /* reverse BEMF*/
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
				1, /* reverse BEMF*/
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

};



#define DEFAULT_TRAIN_CFG(_EN, _R, _P, _S, _LL, _LR)  { \
						{ /* pidctl_config_t*/ \
								500, 180, 500,  /* kP, kI, kD */ \
						}, \
						{ /* inertia_config_t */ \
								120, 120		/* dec, acc */ \
						}, \
						vpolicy_normal,  /*vpolicy_normal, vpolicy_pure_volt, vpolicy_pure_pwm,*/ \
						0, /* enable_inertia */		\
						_EN, /* enabled */			\
						1, /* enable_pid */			\
						0, /* notify_speed */		\
						0, /* notify_pose */		\
						0, /* bemfIIR; */			\
						0, /* postIIR */			\
						0, /* fix_bemf; */			\
						0,  /*	uint8_t en_spd2pow; */\
						20, /*	uint8_t min_power; */ \
						_R, /* reversed */			\
                        _S, /* slipping */          \
						_P, /* pose_per_cm */       \
                        _LL, /* len right*/         \
                        _LR, /* len left */         \
				}

static  train_config_t Trains[NUM_TRAINS] = {
		//DEFAULT_TRAIN_CFG(1,0, 450, 115, 12, 6), // 8821 (V221) with 2 freight cars
		DEFAULT_TRAIN_CFG(1,0, 450, 120, 6,  18), // 8821 (V221) with 3 freight cars
        DEFAULT_TRAIN_CFG(1,0, 310, 120, 15,  2), // 8805 with 4 wine cars - 120 with 3 wine cars
        DEFAULT_TRAIN_CFG(0,0, 500, 100, 10, 10),
        DEFAULT_TRAIN_CFG(0,0, 500, 100, 10, 10),
		DEFAULT_TRAIN_CFG(0,0, 500, 100, 10, 10),
        DEFAULT_TRAIN_CFG(0,0, 500, 100, 10, 10),
        DEFAULT_TRAIN_CFG(0,0, 500, 100, 10, 10),
        DEFAULT_TRAIN_CFG(0,0, 500, 100, 10, 10),
};


static const turnout_config_t Turnouts[NUM_TURNOUTS] = {
#ifndef TRAIN_SIMU
        {TURN1A_GPIO_Port, TURN1A_Pin, TURN1B_Pin},
        {TURN2A_GPIO_Port, TURN2A_Pin, TURN2B_Pin},
        {TURN3A_GPIO_Port, TURN3A_Pin, TURN3B_Pin},
        //{TURN4A_GPIO_Port, TURN4A_Pin, TURN4B_Pin},
#else
        {NULL, 1, 2}
#endif

};




static int setup_done = 1;

const canton_config_t *get_canton_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_CANTONS)) return NULL;
	return &Cantons[idx];
}


const train_config_t *get_train_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TRAINS)) return NULL;
	return &Trains[idx];
}


const turnout_config_t  *get_turnout_cnf(int idx)
{
	if (!setup_done) return config_error(ERR_SETUP_KO, "railconfig_setup_default not called");
	if ((idx<0) || (idx>= NUM_TURNOUTS)) return NULL;
	return &Turnouts[idx];
}



static const led_config_t _led_conf[CONFIG_NLED] = {
#ifndef TRAIN_SIMU
    { LED0_GPIO_Port, LED0_Pin}
#else
    { NULL, 0}
#endif
};

const led_config_t *get_led_cnf(int idx)
{
    if (idx<0) return NULL;
    if (idx>CONFIG_NLED) return NULL;
    return &_led_conf[idx];
}
