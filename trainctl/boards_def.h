/*
 * boards.h
 *
 *  Created on: Nov 23, 2020
 *      Author: danielbraun
 */

#ifndef INC_TRAINCTL_CONFIG_H_
#error please include Core/Inc/trainctl_config.h first
#endif

/*
 definition of each feature to include for a given build (typ. a given board)
 the build is defined in trainctl_config.h (outside trainctl directory)
 */

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
#define BOARD_HAS_SERVOS
#define BOARD_HAS_INA3221   // must be set for stats
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
#define BOARD_HAS_FLASH
#define BOARD_CAN_BE_MASTER


#endif


#ifdef TRN_BOARD_UNIT_TEST

#define BOARD_NUMBER 0
#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_SERVOS
#define BOARD_HAS_INA3221   // must be set for stats
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
#define BOARD_HAS_FLASH
#define BOARD_CAN_BE_MASTER


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
#define BOARD_HAS_FLASH
#define BOARD_CAN_BE_MASTER

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
////#define BOARD_HAS_SERVOS
#define BOARD_HAS_INA3221
#define BOARD_HAS_CTRL
#define BOARD_HAS_TOPOLOGY
#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
#define BOARD_HAS_FLASH
#define BOARD_CAN_BE_MASTER

#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
#ifdef TRN_BOARD_MAINV0
#define BOARD_HAS_ROTARY_ENCODER
#else
#define BOARD_HAS_TWO_BUTTONS
#endif

#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
#define BOARD_HAS_TRKPLN
#define BOARD_HAS_OSCILLO

#define SSD1306_I2C_PORTS_DECL extern I2C_HandleTypeDef hi2c3
#define SSD1306_I2C_PORTS { &hi2c3 }
#define INA3221_I2C_PORT (hi2c1)

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

#if defined(TRN_BOARD_G4MASTER1)
#define STM32_G4
#define STM32G4

#define BOARD_NUMBER 0

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_SERVOS
#define BOARD_HAS_INA3221
#define BOARD_HAS_CTRL
#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
#define BOARD_HAS_LPUART
//#define BOARD_HAS_USB_HOST
#define BOARD_HAS_FLASH
#define BOARD_CAN_BE_MASTER

#define BOARD_HAS_CAN
#define BOARD_HAS_FDCAN
#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
//#define BOARD_HAS_TWO_BUTTONS

#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
#define BOARD_HAS_TRKPLN
#define BOARD_HAS_OSCILLO

#define SSD1306_I2C_PORTS_DECL extern I2C_HandleTypeDef hi2c3
#define SSD1306_I2C_PORTS { &hi2c3 }
#define INA3221_I2C_PORT (hi2c1)

/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */
//#define NUM_LOCAL_CANTONS_HW 6
//#define NUM_LOCAL_CANTONS_SW 8


//#define NUM_LOCAL_TURNOUTS 8

#define CAN_DEVICE hfdcan1



#endif // TRN_BOARD_GSLV1

// -----------------------------------------

#if defined(TRN_BOARD_G4SLV1)
#define STM32_G4
#define STM32G4

#define BOARD_NUMBER 0

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_SERVOS
#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
#define BOARD_HAS_LPUART
//#define BOARD_HAS_USB_HOST
//#define BOARD_HAS_FLASH
//#define BOARD_CAN_BE_MASTER

#define BOARD_HAS_CAN
#define BOARD_HAS_FDCAN
//#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
//#define BOARD_HAS_TWO_BUTTONS

//#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN
//#define BOARD_HAS_OSCILLO

#define SSD1306_I2C_PORTS_DECL extern I2C_HandleTypeDef hi2c3
#define SSD1306_I2C_PORTS { &hi2c3 }
#define INA3221_I2C_PORT (hi2c1)

/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */
//#define NUM_LOCAL_CANTONS_HW 6
//#define NUM_LOCAL_CANTONS_SW 8


//#define NUM_LOCAL_TURNOUTS 8

#define CAN_DEVICE hfdcan1



#endif // TRN_BOARD_GSLV1

// -----------------------------------------
// -----------------------------------------

#ifdef TRN_BOARD_DISPATCHER
#define STM32_F103
#define STM32F1

#define BOARD_NUMBER 1 //xxx

#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
//#define BOARD_HAS_SERVOS
#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
//#define BOARD_HAS_FLASH
#define BOARD_HAS_CAN
//#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
//#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN
// #define BOARD_CAN_BE_MASTER


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

#define BOARD_NUMBER 1 //XXX to be removed

//#define BOARD_HAS_CANTON
#define BOARD_HAS_TURNOUTS
#define BOARD_HAS_SERVOS
//#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
//#define BOARD_HAS_FLASH
#define BOARD_HAS_CAN
//#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
//#define BOARD_HAS_UI_GEN
//#define BOARD_HAS_UI_CTC
#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN
// #define BOARD_CAN_BE_MASTER

//#define SSD1306_I2C_PORTS_DECL extern I2C_HandleTypeDef hi2c1
//#define SSD1306_I2C_PORTS        { &hi2c1 }

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

#ifdef TRN_BOARD_UI
#define STM32_F103
#define STM32F1

#define BOARD_NUMBER 1 //xxx

//#define BOARD_HAS_CANTON
//#define BOARD_HAS_TURNOUTS
//#define BOARD_HAS_SERVOS
//#define BOARD_HAS_INA3221
//#define BOARD_HAS_CTRL
//#define BOARD_HAS_TOPOLOGY
//#define BOARD_HAS_USB
//#define BOARD_HAS_USB_HOST
//#define BOARD_HAS_FLASH
#define BOARD_HAS_CAN
#define BOARD_HAS_IHM
//#define BOARD_HAS_ROTARY_ENCODER
#define BOARD_HAS_UI_GEN
#define BOARD_HAS_UI_CTC
//#define BOARD_HAS_LED
//#define BOARD_HAS_TRKPLN
// #define BOARD_CAN_BE_MASTER


#define SSD1306_I2C_PORTS_DECL extern I2C_HandleTypeDef hi2c1
#define SSD1306_I2C_PORTS { &hi2c1 }

/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */

#define CAN_DEVICE hcan

// -----------------------------------------


#endif // TRN_BOARD_UI
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
