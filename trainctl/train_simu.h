#ifndef TRAIN_SIMU_H
#define TRAIN_SIMU_H


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#define TRN_BOARD_MAIN

#define BOARD_NUMBER 0

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
//#define BOARD_HAS_INA3221
#define BOARD_HAS_CTRL
#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
//#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
#define BOARD_HAS_LED
#define BOARD_HAS_TRKPLN

void train_simu_canton_volt(int numcanton, int voltidx, int vlt100);
void train_simu_canton_set_pwm(int numcanton, int8_t dir, int duty);

extern uint32_t SimuTick;
#define HAL_GetTick() (SimuTick)

//#define NUM_LOCAL_CANTONS 8
#define NUM_LOCAL_CANTONS_HW 5
#define NUM_LOCAL_CANTONS_SW 6 // XXX

#define NUM_LOCAL_TURNOUTS 8


#define USE_INA3221

#define NUM_TRAINS 8

#define NOTIF_VOFF 0

#include "low/canton_bemf.h"
#include "low/canton.h"
#include "low/turnout.h"
#include "spdctl/spdctl.h"
#include "msg/trainmsg.h"
#include "ctrl/ctrl.h"


#define GPIOA NULL
#define GPIOB NULL
#define GPIOC NULL
#define GPIOD NULL
#define GPIOE NULL

#define GPIO_PIN_0 (1<<0)
#define GPIO_PIN_1 (1<<1)
#define GPIO_PIN_2 (1<<2)
#define GPIO_PIN_3 (1<<3)
#define GPIO_PIN_4 (1<<4)
#define GPIO_PIN_5 (1<<5)
#define GPIO_PIN_6 (1<<6)
#define GPIO_PIN_7 (1<<7)
#define GPIO_PIN_8 (1<<8)
#define GPIO_PIN_9 (1<<9)
#define GPIO_PIN_10 (1<<10)
#define GPIO_PIN_11 (1<<11)
#define GPIO_PIN_12 (1<<12)
#define GPIO_PIN_13 (1<<13)
#define GPIO_PIN_14 (1<<14)
#define GPIO_PIN_15 (1<<15)

#define TIM_CHANNEL_1 1
#define TIM_CHANNEL_2 2
#define TIM_CHANNEL_3 3
#define TIM_CHANNEL_4 4
#endif
