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

 int disable_ina3221 = 0; // global disable (when debugging something else)

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

uint8_t ina3221_devices[4] = {0, 0, 0, 0};  // 1 if device is present
static uint16_t ina_conf_val = 0;

static void ina3221_configure(int a, int continuous)
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
			| INA3221_CONF_MODE_SHUNT;
    if (continuous) w16 |= INA3221_CONF_MODE_CONTINUOUS;
	ina3221_write16(a, INA3221_REG_CONFIG, w16);
	ina_conf_val = w16;
    if ((0)) osDelay(100*1);

    //if ((0)) ina3221_start_read();

    return;
}

// ----------------------------------------------------------------------------



void ina3221_trigger_conversion(void)
{
	if (disable_ina3221) return;
	uint32_t t0trig = GetCurrentMicro();
	for (int dev = 0; dev<3; dev++) {
		if (!ina3221_devices[dev]) continue;
		int addr = 0x40 + dev;
		ina3221_write16(addr, INA3221_REG_CONFIG, ina_conf_val);
		itm_debug1(DBG_INA3221, "ina/Tr", addr);
	}
	uint32_t ttrig = GetCurrentMicro() - t0trig;
	itm_debug2(DBG_INA3221, "ina trg", ttrig, ina3221_errors);
}

void ina3221_init(int continuous)
{
	if (disable_ina3221) return;
	//I2C_Scan();
	for (int dev = 0; dev<4; dev++) {
		int addr = 0x40 + dev;
	    HAL_StatusTypeDef res;
        res = HAL_I2C_IsDeviceReady(&INA3221_I2C_PORT, addr << 1, 1, 10);
        if (res == HAL_OK) {
        	ina3221_configure(addr, continuous);
        	ina3221_devices[dev]=1;
        	itm_debug1(DBG_INA3221, "INA@", addr);
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


static uint32_t t0read;
static uint32_t t1;
uint32_t ina3221_scan_dur = 0;
uint32_t ina3221_inter_dur = 0;

extern void Error_Handler(void);

static void _err(void)
{
	if ((1)) Error_Handler();
}

void ina3221_start_read(int16_t *vals, uint8_t *flagdone)
{
	if (get_reg_step != -1) {
		itm_debug1(DBG_INA3221|DBG_ERR, "ina rd ko", get_reg_step);
		_err();
		return;
	}
	pflagdone = flagdone;
	if (pflagdone) *pflagdone = 0;

	if ((0)) { // TODO remove
		uint16_t t = ina3221_read16(0x40, INA3221_REG_MASK_ENABLE);
		int cvr = t & 0x1;
		itm_debug2(DBG_INA3221, "msk/en", t, cvr);
	}
	pvalues = vals;

	itm_debug1(DBG_INA3221, "ina rd", ina3221_errors);
	//t0 = HAL_GetTick();
	t0read = GetCurrentMicro();
	ina3221_inter_dur = t0read - t1;
	itm_debug1(DBG_INA3221, "ina inter", ina3221_inter_dur);
	get_reg_step = 0;
	_get_next_reg();
}

static void _get_next_reg(void)
{
	for (;;) {
		if (get_reg_step<0) {
			itm_debug1(DBG_INA3221|DBG_ERR, "bad get_reg_step", get_reg_step);
		}
		if (get_reg_step == INA3221_NUM_VALS) {
			if (pflagdone) *pflagdone = 1;
			uint32_t tm = GetCurrentMicro();
			ina3221_scan_dur = tm - t0read;
			t1 = tm;
			get_reg_step = -1;
			itm_debug2(DBG_INA3221,"ina done", ina3221_scan_dur, tm);
			return;
		}
		int numdev = get_reg_step/3;
		if (!ina3221_devices[numdev]) {
			pvalues[get_reg_step] = 0xFF;
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
			itm_debug1(DBG_INA3221|DBG_ERR, "readit", status);
			_err();
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
	itm_debug2(DBG_INA3221|DBG_ERR, "i2c err", hi2c->ErrorCode, get_reg_step);
	get_reg_step = -1;
	if ((1)) disable_ina3221 = 1;
	if ((0)) _err();
}

