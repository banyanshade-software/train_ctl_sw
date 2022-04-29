/*
 * boards.h
 *
 *  Created on: Nov 23, 2020
 *      Author: danielbraun
 */

#ifndef INC_TRAINCTL_CONFIG_H_
#error please include Core/Inc/trainctl_config.h first
#endif


#ifndef INC_TBOARDS_H_
#define INC_TBOARDS_H_



// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------


#ifdef TRN_BOARD_SIMU

#define BOARD_NUMBER 0
#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
//#define BOARD_HAS_INA3221
#define BOARD_HAS_CTRL
#define BOARD_HAS_TOPOLOGY
#define BOARD_HAS_USB
//#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
//#define BOARD_HAS_UI_GEN
#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
#define BOARD_HAS_TRKPLN
//#define BOARD_HAS_OSCILLO
//#define BOARD_HAS_USB_HOST


#endif

// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------



#ifdef TRN_BOARD_MAIN_ZERO

#define STM32_F4
#define STM32F4

#define BOARD_HAS_CAN
#define BOARD_NUMBER 0

#define CAN_DEVICE hcan1
#define BOARD_HAS_USB_HOST

#define OAM_ONLY 1

#endif // TRN_BOARD_MAIN_ZERO


// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------


#ifdef TRN_BOARD_MAIN
#error obsolete, use MAINV0 or MAINV04
#endif

#if defined(TRN_BOARD_MAINV0) || defined(TRN_BOARD_MAINV04)
#define STM32_F4
#define STM32F4

#define BOARD_NUMBER 0

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_INA3221
#define BOARD_HAS_CTRL
#define BOARD_HAS_TOPOLOGY
#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
#define BOARD_HAS_ROTARY_ENCODER
#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
#define BOARD_HAS_TRKPLN
#define BOARD_HAS_OSCILLO


/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */
//#define NUM_LOCAL_CANTONS_HW 6
//#define NUM_LOCAL_CANTONS_SW 8


//#define NUM_LOCAL_TURNOUTS 8

#define CAN_DEVICE hcan1

// -----------------------------------------


#endif // TRN_BOARD_MAIN


// -----------------------------------------

#ifdef TRN_BOARD_DISPATCHER
#define STM32_F103
#define STM32F1

#define BOARD_NUMBER 1 //xxx

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
#define BOARD_HAS_CAN
//#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
//#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN

/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */


#define CAN_DEVICE hcan

// -----------------------------------------


#endif // TRN_BOARD_DISPATCHER

// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------

#ifdef TRN_BOARD_SWITCHER
#define STM32_F103
#define STM32F1

#define BOARD_NUMBER 1 //xxx

//#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
//#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
//#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN

#define SSD1306_I2C_PORT        hi2c1

/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */

#define CAN_DEVICE hcan

// -----------------------------------------


#endif // TRN_BOARD_SWITCHER

// -----------------------------------------
// -----------------------------------------
// -----------------------------------------
// -----------------------------------------



#ifdef BOARD_HAS_IHM
#define BOARD_HAS_TFT
#endif

// common config, should be moved out



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




// ---------------------------------------------------------

// max values for complete system
#define MAX_TOTAL_TURNOUTS 	64




#endif /* INC_TBOARDS_H_ */
