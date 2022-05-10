/*
 * ina3221_config.h
 *
 *  Created on: Apr 13, 2021
 *      Author: danielbraun
 */

#include "trainctl_config.h"

#ifndef INA3221_INA3221_CONFIG_H_
#define INA3221_INA3221_CONFIG_H_





#ifndef INA3221_I2C_PORT
#error define it in board_Def.h
#define INA3221_I2C_PORT (hi2c3)
#endif

extern I2C_HandleTypeDef INA3221_I2C_PORT;



#endif /* INA3221_INA3221_CONFIG_H_ */
