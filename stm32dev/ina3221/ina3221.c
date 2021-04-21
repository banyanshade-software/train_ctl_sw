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
//#include "utils/microsec.h"

#include "railconfig.h" // for ugly hack

uint16_t ina3221_errors = 0;
extern uint32_t GetCurrentMicro(void);


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

// ----------------------------------------------------------------------------

static void ina3221_configure(int a)
{
	//HAL_StatusTypeDef status;
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

    if ((0)) osDelay(100*1);

    //if ((0)) ina3221_start_read();

    return;
}

// ----------------------------------------------------------------------------


static uint8_t ina3221_devices[4] = {0, 0, 0, 0};  // 1 if device is present


void ina3221_init(void)
{
	//I2C_Scan();
	for (int dev = 0; dev<3; dev++) {
		int addr = 0x40 + dev;
	    HAL_StatusTypeDef res;
        res = HAL_I2C_IsDeviceReady(&INA3221_I2C_PORT, addr << 1, 1, 10);
        if (res == HAL_OK) {
        	ina3221_configure(addr);
        	ina3221_devices[dev]=1;
        	itm_debug1("INA@", addr);
        } else {
        	ina3221_devices[dev]=0;
        }
	}
}
// ----------------------------------------------------------------------------


static int16_t *pvalues = NULL;
static uint8_t *pflagdone = NULL;

static int get_reg_step = -1;
static void _get_next_reg(void);


static uint32_t t0;
static uint32_t t1;
uint32_t ina3221_scan_dur = 0;
uint32_t ina3221_inter_dur = 0;



void ina3221_start_read(int16_t *vals, uint8_t *flagdone)
{
	if (get_reg_step != -1) {
		itm_debug1("ina rd ko", get_reg_step);
		return;
	}
	pflagdone = flagdone;
	if (pflagdone) *pflagdone = 0;

	pvalues = vals;

	itm_debug1("ina rd", ina3221_errors);
	//t0 = HAL_GetTick();
	t0 = GetCurrentMicro();
	ina3221_inter_dur = t0 - t1;
	get_reg_step = 0;
	_get_next_reg();
}

static void _get_next_reg(void)
{
	for (;;) {
		if (get_reg_step<0) {
			itm_debug1("bad get_reg_step", get_reg_step);
		}
		if (get_reg_step == INA3221_NUM_VALS) {
			if (pflagdone) *pflagdone = 1;
			uint32_t tm = GetCurrentMicro();
			ina3221_scan_dur = tm - t0;
			t1 = tm;
			get_reg_step = -1;
			return;
		}
		int numdev = get_reg_step/3;
		if (!ina3221_devices[numdev]) {
			pvalues[get_reg_step] = 0;
			get_reg_step++;
			continue;
		}
		int reg;
		switch (get_reg_step % 3) {
		case 0: reg = INA3221_REG_CH1_SHUNTVOLT; break;
		case 1: reg = INA3221_REG_CH2_SHUNTVOLT; break;
		case 2: reg = INA3221_REG_CH3_SHUNTVOLT; break;
		}
		HAL_StatusTypeDef status;
		int addr = 0x40 + numdev;
		status = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, addr<<1, reg, I2C_MEMADD_SIZE_8BIT,
	    		(uint8_t *)&pvalues[get_reg_step], 2);
		if (status != HAL_OK) {
			itm_debug1("readit", status);
		}
		get_reg_step++;
		break;
	}
}


void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	_get_next_reg();

}



void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	ina3221_errors++;
	itm_debug2("i2c err", hi2c->ErrorCode, get_reg_step);
	get_reg_step = -1;
}


#if 0
//int16_t ina3221_values[3];
//int16_t ina3221_prev[3];
uint32_t ina3221_nscan = 0;
uint32_t ina3221_scan_dur = 0;
uint32_t ina3221_inter_dur = 0;
uint32_t ina3221_errcnt = 0;
static uint32_t t0;
static uint32_t t1;



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
#if 1

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

#endif

