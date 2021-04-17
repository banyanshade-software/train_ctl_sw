/*
 * ina3221.c
 *
 *  Created on: Apr 13, 2021
 *      Author: danielbraun
 */


#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#include "ina3221.h"
#include "ina3221_def.h"
#include "ina3221_config.h"


#include "railconfig.h" // for ugly hack

uint16_t ina3221_errors = 0;

static uint16_t ina3221_read16(int a, int reg)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
    status = HAL_I2C_Mem_Read(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) {
    	ina3221_errors++;
    	return 0;
    } else {
    	return __builtin_bswap16(w16);
    }
}

static void ina3221_write16(int a, int reg, uint16_t v)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
	if ((1)) w16 = __builtin_bswap16(v);
	else w16=v;
    status = HAL_I2C_Mem_Write(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) {
       	ina3221_errors++;
    }
}

int16_t ch1vs;
int16_t ch2vs;
int16_t ch3vs;

static uint16_t addr = 0;

static void i2c_ready(int a)
{
	addr = a;
	HAL_StatusTypeDef status;
	uint16_t w16;

    uint16_t mid = ina3221_read16(a, INA3221_REG_MANUFACTURER_ID);
     // 0x5449
    uint16_t did = ina3221_read16(a,  INA3221_REG_DIE_ID);
    // 0x3220


    if ((1)) {
    	w16 = INA3221_CONF_RESET;
    	ina3221_write16(a, INA3221_REG_CONFIG, w16);
    	osDelay(100*5);
    	//if ((1)) return;
    }
    w16 = INA3221_CONF_CH1_EN | INA3221_CONF_CH2_EN | INA3221_CONF_CH3_EN
    		| INA3221_CONF_VS_CT_140u | INA3221_CONF_AVG1
			| INA3221_CONF_MODE_CONTINUOUS | INA3221_CONF_MODE_SHUNT;
	ina3221_write16(a, INA3221_REG_CONFIG, w16);

    osDelay(100*1);
    //int16_t i1 = ina3221_read16(a, INA3221_REG_CH1_BUSVOLT);
    //int16_t i2 = ina3221_read16(a, INA3221_REG_CH2_BUSVOLT);
    //int16_t i3 = ina3221_read16(a, INA3221_REG_CH3_BUSVOLT);

    ina3221_start_read();

    return;
    //uint8_t buf[4];
    //status = HAL_I2C_Mem_Read(&INA3221_I2C_PORT, a<<1, INA3221_REG_CH1_BUSVOLT, I2C_MEMADD_SIZE_8BIT, buf, 4, HAL_MAX_DELAY);
    //int16_t it = i1+i2+i3;
    HAL_StatusTypeDef s1,s2, s3;
    //HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
    //uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
    s1 = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, a<<1, INA3221_REG_CH1_SHUNTVOLT, I2C_MEMADD_SIZE_8BIT,
    		(uint8_t *)&ch1vs, 2);
    s2 = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, a<<1, INA3221_REG_CH2_SHUNTVOLT, I2C_MEMADD_SIZE_8BIT,
    		(uint8_t *)&ch1vs, 2);
    s3 = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, a<<1, INA3221_REG_CH3_SHUNTVOLT, I2C_MEMADD_SIZE_8BIT,
    		(uint8_t *)&ch1vs, 2);

    //int16_t it = i1+i2+i3;
}

int16_t ina3221_values[3];
int16_t ina3221_prev[3];
uint32_t ina3221_nscan = 0;
uint32_t ina3221_scan_dur = 0;
uint32_t ina3221_inter_dur = 0;
uint32_t ina3221_errcnt = 0;
static uint32_t t0;
static uint32_t t1;

uint32_t GetCurrentMicro(void);

#if 1
static int get_reg_step = -1;
static void _get_next_reg(void)
{
	int reg;
	switch (get_reg_step) {
	case 0:
		reg = INA3221_REG_CH1_SHUNTVOLT;
		break;
	case 1:
		reg = INA3221_REG_CH2_SHUNTVOLT;
		break;
	case 2:
		reg = INA3221_REG_CH3_SHUNTVOLT;
		break;
	case 3:
		ina3221_nscan++;
		//ina3221_scan_dur = HAL_GetTick() - t0;
		uint32_t tm = GetCurrentMicro();
		ina3221_scan_dur = tm - t0;
		t1 = tm;
		for (int i=0 ;i<3; i++) {
#if 0
			ina3221_values[i]=__builtin_bswap16(ina3221_values[i]);
#else
			//ugly hack
			ina3221_prev[i] = ina3221_values[i];
			ina3221_values[i] =__builtin_bswap16(ina3221_values[i]);

			USE_TRAIN(0)	// tconf tvars
			int16_t v = tvars->target_speed;
			if (i==1) {
				if ((abs(ina3221_prev[i])<5000) && (abs(ina3221_values[i])>=5000) && (v>0)) {
					tvars->target_speed = -v;
					itm_debug1("inv1", tvars->target_speed);
					flash_led();
				}
			} else if (i==0) {
				if ((abs(ina3221_prev[i])<5000) && (abs(ina3221_values[i])>=5000) && (v<0)) {
					tvars->target_speed = -v;
					itm_debug1("inv0", tvars->target_speed);
					flash_led();
				}
			}
			ina3221_values[i]=__builtin_bswap16(ina3221_values[i]);
#endif
			get_reg_step = -1;
			itm_debug3("i", ina3221_values[0],ina3221_values[1],ina3221_values[2]);

		}
		return;
	default:
		return;
	}
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, addr<<1, reg, I2C_MEMADD_SIZE_8BIT,
    		(uint8_t *)&ina3221_values[get_reg_step], 2);
	get_reg_step++;
}

void ina3221_start_read(void)
{
	if (get_reg_step != -1) {
		itm_debug1("ina rd ko", get_reg_step);
		return;
	}
	itm_debug1("ina rd", ina3221_errcnt);
	//t0 = HAL_GetTick();
	t0 = GetCurrentMicro();
	ina3221_inter_dur = t0 - t1;
	get_reg_step = 0;
	_get_next_reg();
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	_get_next_reg();

}
#else
// polling version for test
void ina3221_start_read(void)
{
	ina3221_values[0] = ina3221_read16(addr, INA3221_REG_CH1_SHUNTVOLT);
	ina3221_values[1] = ina3221_read16(addr, INA3221_REG_CH2_SHUNTVOLT);
	ina3221_values[2] = ina3221_read16(addr, INA3221_REG_CH3_SHUNTVOLT);
	return;
	uint16_t  w16 = INA3221_CONF_CH1_EN | INA3221_CONF_CH2_EN | INA3221_CONF_CH3_EN
	    		| INA3221_CONF_VS_CT_1m | INA3221_CONF_AVG1
				|INA3221_CONF_MODE_SINGLE /*| INA3221_CONF_MODE_CONTINUOUS*/ | INA3221_CONF_MODE_SHUNT;
	ina3221_write16(addr, INA3221_REG_CONFIG, w16);

}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{

}
#endif


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	ina3221_errcnt++;
    itm_debug2("i2c err", hi2c->ErrorCode, get_reg_step);
    get_reg_step = -1;
}
static void I2C_Scan(void)
{
    HAL_StatusTypeDef res;
    for(uint16_t i = 1; i < 128; i++) {
        res = HAL_I2C_IsDeviceReady(&INA3221_I2C_PORT, i << 1, 1, 10);
        if(res == HAL_OK) {
        	i2c_ready(i);
        } else {
        }
    }
}

void ina3221_init(void)
{
	I2C_Scan();
}
