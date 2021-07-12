/*
 * trainctl_config.h
 *
 *  Created on: Nov 23, 2020
 *      Author: danielbraun
 */

#ifndef INC_TRAINCTL_CONFIG_H_
#define INC_TRAINCTL_CONFIG_H_

//#define TRAINCTL_NEW 1
#define STM32_F4

/*
 * number of local block per board
 * NUM_LOCAL_CANTONS_HW : really configured blocked (used for ADC)
 * NUM_LOCAL_CANTONS_SW : max available in software
 */
#define NUM_LOCAL_CANTONS_HW 5
#define NUM_LOCAL_CANTONS_SW 8

#define NUM_TRAINS	8

#define TFT_DISP 1

/*#define USE_INA3221 	1		// must be 1, was used when switching from ADC reading
#define DISABLE_INA3221 0		// disable INA3221

#define INA3221_USE_DMA 0		// do not use DMA, untested
#define INA3221_USE_IT  1

#define INA3221_TASK	1		// use high prio task instead of IT processing
//#define INA3221_TASKRD	1		// separate tick for reading

#define NOTIF_VOFF 1
*/

#endif /* INC_TRAINCTL_CONFIG_H_ */
