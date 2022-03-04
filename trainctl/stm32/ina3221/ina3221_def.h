/*
 * ina3221_def.h
 *
 *  Created on: Apr 15, 2021
 *      Author: danielbraun
 */

#ifndef INA3221_DEF_H_
#define INA3221_DEF_H_



// INA3221 registers

#define INA3221_REG_CONFIG          	(0x00)

#define INA3221_REG_CH1_SHUNTVOLT 	   	(0x01)
#define INA3221_REG_CH1_BUSVOLT 		(0x02)
#define INA3221_REG_CH2_SHUNTVOLT 	   	(0x03)
#define INA3221_REG_CH2_BUSVOLT 		(0x04)
#define INA3221_REG_CH3_SHUNTVOLT 	   	(0x05)
#define INA3221_REG_CH3_BUSVOLT 		(0x06)

#define INA3221_REG_CH1_CRIT_LIM 		(0x07)
#define INA3221_REG_CH1_WARN_LIM 		(0x08)
#define INA3221_REG_CH2_CRIT_LIM 		(0x09)
#define INA3221_REG_CH2_WARN_LIM 		(0x0A)
#define INA3221_REG_CH3_CRIT_LIM 		(0x0B)
#define INA3221_REG_CH3_WARN_LIM 		(0x0C)

#define INA3221_REG_SHUNT_VOLT_SUM 		(0x0D)
#define INA3221_REG_SHUNT_VOLT_SUM_LIM 	(0x0E)

#define INA3221_REG_MASK_ENABLE		 	(0x0F)
#define INA3221_REG_POW_VALID_UPPER		(0x10)
#define INA3221_REG_POW_VALID_LOWER		(0x11)


#define INA3221_REG_MANUFACTURER_ID   	(0xfe)
#define INA3221_REG_DIE_ID 	          	(0xff)

// configuration bits

// Configuration Register bits
#define INA3221_CONF_RESET			(1 << 15)

#define INA3221_CONF_CH1_EN         (1 << 14)
#define INA3221_CONF_CH2_EN         (1 << 13)
#define INA3221_CONF_CH3_EN         (1 << 12)

#define INA3221_CONF_CH3_AVG_B2		(1 << 11)
#define INA3221_CONF_CH3_AVG_B1		(1 << 10)
#define INA3221_CONF_CH3_AVG_B0		(1 <<  9)

#define INA3221_CONF_VBUS_CT_B2		(1 <<  8)
#define INA3221_CONF_VBUS_CT_B1		(1 <<  7)
#define INA3221_CONF_VBUS_CT_B0		(1 <<  6)

#define INA3221_CONF_VS_CT_B2		(1 <<  5)
#define INA3221_CONF_VS_CT_B1		(1 <<  4)
#define INA3221_CONF_VS_CT_B0		(1 <<  3)

#define INA3221_CONF_MODE_B3		(1 <<  2)
#define INA3221_CONF_MODE_B2		(1 <<  1)
#define INA3221_CONF_MODE_B1		(1 <<  0)

#define INA3221_CONF_AVG1			(0x0 << 9)
#define INA3221_CONF_AVG4			(0x1 << 9)
#define INA3221_CONF_AVG16			(0x2 << 9)
#define INA3221_CONF_AVG64			(0x3 << 9)
#define INA3221_CONF_AVG128			(0x4 << 9)
#define INA3221_CONF_AVG256			(0x5 << 9)
#define INA3221_CONF_AVG512			(0x6 << 9)
#define INA3221_CONF_AVG1024		(0x7 << 9)

// mode bits, or-able
#define INA3221_CONF_MODE_CONTINUOUS	(0x4)
#define INA3221_CONF_MODE_SINGLE		(0x0)

#define INA3221_CONF_MODE_PWR_DOWN		(0x0)
#define INA3221_CONF_MODE_SHUNT         (0x1)
#define INA3221_CONF_MODE_BUSV			(0x2)

// conversion time
#define INA3221_CONF_VS_CT_140u			(0x0 << 3)
#define INA3221_CONF_VS_CT_204u			(0x1 << 3)
#define INA3221_CONF_VS_CT_332u			(0x2 << 3)
#define INA3221_CONF_VS_CT_588u			(0x3 << 3)
#define INA3221_CONF_VS_CT_1m			(0x4 << 3)	//1.1ms (default)
#define INA3221_CONF_VS_CT_2m			(0x5 << 3)  //2.116ms
#define INA3221_CONF_VS_CT_4m			(0x6 << 3)  //4.156ms
#define INA3221_CONF_VS_CT_8m			(0x7 << 3)	//8.244ms

#define INA3221_CONF_VBUS_CT_140u		(0x0 << 6)
#define INA3221_CONF_VBUS_CT_204u		(0x1 << 6)
#define INA3221_CONF_VBUS_CT_332u		(0x2 << 6)
#define INA3221_CONF_VBUS_CT_588u		(0x3 << 6)
#define INA3221_CONF_VBUS_CT_1m			(0x4 << 6)	//1.1ms (default)
#define INA3221_CONF_VBUS_CT_2m			(0x5 << 6)  //2.116ms
#define INA3221_CONF_VBUS_CT_4m			(0x6 << 6)  //4.156ms
#define INA3221_CONF_VBUS_CT_8m			(0x7 << 6)	//8.244ms


#endif /* INA3221_DEF_H_ */
