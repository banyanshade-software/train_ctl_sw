/*
 * trainctl_config.h
 *
 *  Created on: Nov 23, 2020
 *      Author: danielbraun
 */

#ifndef INC_TRAINCTL_CONFIG_H_
#define INC_TRAINCTL_CONFIG_H_

#define TRN_BOARD_MAIN
//#define TRN_BOARD_DISPATCHER
//#define TRN_BOARD_UI

// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------

#ifdef TRN_BOARD_MAIN
#define STM32_F4

#define BOARD_NUMBER 0
#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_INA3221
#define BOARD_HAS_CTRL
#define BOARD_HAS_TOPOLOGY
#define BOARD_HAS_USB
#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
#define BOARD_HAS_LED
#define BOARD_HAS_TRKPLN
/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */
#define NUM_LOCAL_CANTONS_HW 6
#define NUM_LOCAL_CANTONS_SW 8


#define NUM_LOCAL_TURNOUTS 8

#define CAN_DEVICE hcan1

// -----------------------------------------


#endif // TRN_BOARD_MAIN


// -----------------------------------------

#ifdef TRN_BOARD_DISPATCHER
#define STM32_F103

#define BOARD_NUMBER 1 //xxx

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
#define BOARD_HAS_CAN
//#define BOARD_HAS_IHM
#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN
/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */
#define NUM_LOCAL_CANTONS_HW 1
#define NUM_LOCAL_CANTONS_SW 1


#define NUM_LOCAL_TURNOUTS 8

#define CAN_DEVICE hcan

// -----------------------------------------


#endif // TRN_BOARD_MAIN

// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------


#ifdef BOARD_HAS_IHM
#define BOARD_HAS_TFT
#endif
// common config, should be moved out

#define NUM_TRAINS	8


#define USE_INA3221 		1

#define DISABLE_INA3221 	0
#define INA3221_TASK		1
// only valid with INA3221_TASK
#define INA3221_TASKRD 		1
#define INA3221_CHECKCONV 	0
#define INA3221_CONTIUNOUS	1
#define NOTIF_VOFF 0

#ifndef OAM_ONLY
#define OAM_ONLY 0
#endif

#endif /* INC_TRAINCTL_CONFIG_H_ */
