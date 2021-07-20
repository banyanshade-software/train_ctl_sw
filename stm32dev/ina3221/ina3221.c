/*
 * ina3221.c
 *
 *  Created on: Apr 13, 2021
 *      Author: danielbraun
 */


#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "cmsis_os2.h"

#include "ina3221.h"
#include "ina3221_def.h"
#include "ina3221_config.h"
//#include "utils/microsec.h"

#include "misc.h"
#include "../../trainctl/msg/trainmsg.h"

// ----------------------------------------------------------------------------------
// global run mode, each tasklet implement this
static runmode_t run_mode = 0;
static uint8_t testerAddr;

// ----------------------------------------------------------------------------------


uint8_t ina3221_devices[4] = {0, 0, 0, 0};  // 1 if device is present
//static uint16_t ina_conf_val = 0;

uint16_t ina3221_errors = 0;
static uint8_t disable_ina3221 = 0;
static int ina3221_init_done = 0;
static int ina_conf_val = 0;

#ifndef INA3221_TASKRD
#error hu?
#endif

static void bkpoint(int loc, int err)
{
	itm_debug2(DBG_ERR|DBG_INA3221, "INA ERR", loc, err);
}

_UNUSED_ static int ina3221_write16it(int a, int reg, uint16_t v);

static int ina3221_write16(int a, int reg, uint16_t v);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT_NW(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT_NW(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);


static void ina3221_init_and_configure(void);

// ----------------------------------------------------------------------------------

#if INA3221_TASK

#if INA3221_CONTIUNOUS
#if INA3221_TASKRD == 0
#error INA3221_TASKRD should be set
#endif
#if INA3221_CHECKCONV
#error INA3221_CHECKCONV shoud be zero
#endif
#endif

static uint16_t values[INA3221_NUM_VALS*2] = {0};
uint16_t *cur_values = values;
uint16_t *prev_values = values+12; // TODO : double buffer unused

extern osThreadId_t ina3221_taskHandle;
static int lastErr = 0;
static int cvrf_dev = 0;

typedef enum {
	state_idle,

	state_trig_0,
	state_trig_1,
	state_trig_2,
	state_trig_3,

#if INA3221_CHECKCONV
	state_chk_cvrf,
#endif

	state_rd_0,
	state_rd_1,
	state_rd_2,
	state_rd_3,
	state_rd_4,
	state_rd_5,
	state_rd_6,
	state_rd_7,
	state_rd_8,
	state_rd_9,
	state_rd_10,
	state_rd_11,

} ina_state_t;

static int _trig(int dev);
static void _reg_read(int dev, int reg);
static void _read_complete(int err);
static int _next_dev(int d);
#if INA3221_CHECKCONV
static int _read_cvrf(void);
#endif

static volatile uint16_t mask_en_val = 0;

static void handle_ina_notif(uint32_t notif);

static 	ina_state_t state = state_idle;

static void run_ina_task(void)
{
	ina3221_init_and_configure();
	_UNUSED_ int nstuck = 0;
	for (;;) {
		uint32_t notif = 0;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		handle_ina_notif(notif);

		for (;;) {
			msg_64_t m;
			int rc = mqf_read_to_ina3221(&m);
			if (rc) break;
			switch (m.cmd) {
			case CMD_RESET:
				// FALLTHRU
			case CMD_EMERGENCY_STOP:
				// TODO
				continue;
				break;
			case CMD_SETRUN_MODE:
				if (run_mode != m.v1u) {
					run_mode = m.v1u;
					testerAddr = m.from;
				}
				continue;
				break;
			default:
				break;
			}
		}
	}
}

static void handle_ina_notif(uint32_t notif)
{
	int rc;
	if (notif & NOTIF_INA_TRIG) {
#if INA3221_CONTIUNOUS
		// just ignore
#else
		if (state_idle == state) {
			nstuck = 0;
			itm_debug1(DBG_INA3221, "-TRG", 0);
			int dev = _next_dev(-1);
			if (dev >= 0) {
				state = dev + state_trig_0;
				if (cur_values == values) {
					cur_values = values + 12;
					prev_values = values;
				} else if (cur_values == values + 12) {
					cur_values = values;
					prev_values = values + 12;
				} else {
					bkpoint(0, 1001);
				}
				rc = _trig(dev);
				if (rc) {
					state = state_idle;
					continue;
				}
			} else {
				itm_debug1(DBG_INA3221, "TRG/N", 0);
#if INA3221_TASKRD
#else
	_read_complete(0);
#endif
state = state_idle;
			}
		} else {
			nstuck++;
			if (nstuck>3) {
				bkpoint(0, 1000);
				itm_debug1(DBG_ERR|DBG_INA3221, "STUCK", state);
				state = state_idle;
				continue;
			}
		}
#endif // INA3221_CONTIUNOUS
	}
	if (notif & NOTIF_INA_WRCOMPL) {
		itm_debug1(DBG_INA3221, "WRcpl", state);
		if ((state >= state_trig_0) && (state <= state_trig_3)) {
			int dev = state - state_trig_0;
			dev = _next_dev(dev);
			if (dev >= 0) {
				state = dev + state_trig_0;
				rc = _trig(dev);
				if (rc) {
					state = state_idle;
					return;
				}
			} else {
#if INA3221_TASKRD
state = state_idle;
#else
#if INA3221_CHECKCONV
state = state_chk_cvrf;
_read_cvrf();
#else
	dev = _next_dev(-1);
	if (dev >= 0) {
		state = state_rd_0 + dev * 3;
		_reg_read(dev, 0);
	} else {
		_read_complete(0);
		state = state_idle;
	}
#endif
#endif
			}
			/*} else {
                    dev = _next_dev(-1);
                    if (dev >= 0) {
                        state = state_rd_0 + dev * 3;
                        _reg_read(dev, 0);
                    } else {
                        _read_complete(0);
                        state = state_idle;
                    }
                }*/
		} else {
			bkpoint(1,1000);
		}
	}
	if (notif & NOTIF_INA_READ) {
#if INA3221_TASKRD
#if INA3221_CHECKCONV
		state = state_chk_cvrf;
		_read_cvrf();
#else
		int dev = _next_dev(-1);
		if (dev >= 0) {
			state = state_rd_0 + dev * 3;
			_reg_read(dev, 0);
		} else {
			_read_complete(0);
			state = state_idle;
		}

#endif
#endif
	}
	if (notif & NOTIF_INA_RDCOMPL) {
		itm_debug1(DBG_INA3221, "RDcpl", state);
#if INA3221_CHECKCONV
		if (state == state_chk_cvrf) {
			mask_en_val = __builtin_bswap16(mask_en_val);
			if (mask_en_val & 0x0001) {
				itm_debug2(DBG_INA3221, "samplok", state, mask_en_val);
				int dev = _next_dev(-1);
				if (dev >= 0) {
					state = state_rd_0 + dev * 3;
					_reg_read(dev, 0);
				} else {
					_read_complete(0);
					state = state_idle;
				}
			} else {
				itm_debug2(DBG_INA3221, "again", state, mask_en_val);
				_read_cvrf();
			}
#else
		if (0) {
#endif
		} else if ((state >= state_rd_0) && (state <= state_rd_11)) {
			int reg = (state - state_rd_0) % 3;
			int dev = (state - state_rd_0) / 3;
			if (reg==2){
				dev = _next_dev(dev);
				if (dev >= 0) {
					state = (state_rd_0 + dev) * 3;
					_reg_read(dev, 0);
				} else {
					_read_complete(0);
					state = state_idle;
				}
			} else {
				state++;
				_reg_read(dev, reg+1);
			}
		} else {
			bkpoint(2,1000);
		}
	}
	if (notif & NOTIF_INA_ERR) {
		bkpoint(3, lastErr);
		if ((state >= state_rd_0) && (state <= state_rd_11)) {
			// write error
			// TODO
			state = state_idle;
		} else if ((state >= state_trig_0) && (state <= state_trig_3)) {
			// read error
			// TODO
			state = state_idle;
		} else {
			bkpoint(3,1000);
		}
		itm_debug1(DBG_INA3221|DBG_ERR, "i2c rst", lastErr);
		HAL_I2C_Init(&INA3221_I2C_PORT);
	}
}




static int _next_dev(int dev)
{
	dev = dev+1;
	for (;dev<=3;dev++) {
		if (ina3221_devices[dev]) return dev;
	}
	return -1;
}


static int  _trig(int dev)
{
	if (disable_ina3221) return 0;
	cvrf_dev = dev;
	int addr = 0x40 + dev;
	itm_debug2(DBG_INA3221, "TRIG", dev, addr);
	return ina3221_write16it(addr, INA3221_REG_CONFIG, ina_conf_val);
}
static void _reg_read(int dev, int nreg)
{
	int addr = 0x40 + dev;
	int hwreg;
	switch (nreg) {
	case 0: hwreg = INA3221_REG_CH1_SHUNTVOLT; break;
	case 1: hwreg = INA3221_REG_CH2_SHUNTVOLT; break;
	case 2: hwreg = INA3221_REG_CH3_SHUNTVOLT; break;
	default:
		itm_debug1(DBG_ERR|DBG_INA3221, "hu?", nreg);
		return;
	}
	itm_debug3(DBG_INA3221, "gns read", dev, nreg, hwreg);
	HAL_StatusTypeDef status;

	if (__HAL_I2C_GET_FLAG(&INA3221_I2C_PORT, I2C_FLAG_BUSY) != RESET) {
		itm_debug1(DBG_ERR|DBG_INA3221, "busy", 0);
		// TODO _end_next_reg(1);
		return;
	}

	status = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, addr<<1, hwreg, I2C_MEMADD_SIZE_8BIT,
			(uint8_t *)&cur_values[dev*3+nreg], 2);
	if (status != HAL_OK) {
		itm_debug1(DBG_INA3221|DBG_ERR, "readit", status);
		// TODO
		return;
	}
}

#if INA3221_CHECKCONV
static int _read_cvrf(void)
{
	int addr = 0x40 + cvrf_dev; // dev 0
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, addr<<1, INA3221_REG_MASK_ENABLE, I2C_MEMADD_SIZE_8BIT,
			(uint8_t *)&mask_en_val, 2);
	if (status != HAL_OK) {
		itm_debug1(DBG_INA3221|DBG_ERR, "readits", status);
		// TODO
		return -1;
	}
	return 0;
}
#endif

static void _read_complete(_UNUSED_ int err)
{
	uint16_t *valu = (uint16_t *) cur_values;
	int16_t  *vals = (int16_t *) cur_values;
	static int8_t presence[INA3221_NUM_VALS] = {0};

	for (int i = 0; i<INA3221_NUM_VALS; i++) {
			valu[i] = __builtin_bswap16(valu[i]);
	}
	msg_64_t m;

	switch (run_mode) {
	default:
		break;
	case runmode_detect1:
		itm_debug1(DBG_INA3221, "D/1", vals[1]);
		m.from =  MA_CANTON(localBoardNum, 0);
		m.to = MA_UI(1);
		m.cmd = CMD_INA3221_VAL1;
		m.sub = 1;
		m.v1 = vals[1];
		mqf_write_from_ina3221(&m);
		break;
	case runmode_normal:
		for (int i = 0; i<INA3221_NUM_VALS; i++) {
			itm_debug2(DBG_INA3221, "ina val", i, vals[i]);
			int p = (abs(vals[i])>1000) ? 1 : 0;
			if (p == presence[i]) continue;
			presence[i] = p;
			itm_debug3(DBG_INA3221|DBG_PRES, "PRSCH", i,p, vals[i]);
			// notify change
			m.from = MA_CANTON(localBoardNum, 0);
			m.to = MA_CONTROL();
			m.cmd = CMD_PRESENCE_CHANGE;
			m.sub = i;
			m.v1u = p;
			m.v2 = vals[i];
			mqf_write_from_ina3221(&m);


			static int cnt=0;
			cnt++;
			if ((0) && ((cnt%50)==0)) {
				msg_64_t m;
				static int16_t v[12];
				memcpy(v, vals, 12*2);
				for (int i=0; i<12; i++) {
					if (abs(v[i]) > 400) {
						itm_debug2(DBG_INA3221, "ina big", i, v[i]);
					}
				}
				if ((1)) v[7] = cnt;
				m.from = MA_CANTON(localBoardNum, 0);
				m.to = MA_UI(1);
				m.cmd = CMD_INA3221_REPORT;
				m.v32u = (uint32_t) v;
				mqf_write_from_ina3221(&m);
			}
			break;
		}
	}
}


void HAL_I2C_MemTxCpltCallback(_UNUSED_ I2C_HandleTypeDef *hi2c)
{
	BEGIN_ISR
	BaseType_t higher=0;
	xTaskNotifyFromISR(ina3221_taskHandle, NOTIF_INA_WRCOMPL, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
	END_ISR;
}

void HAL_I2C_MemRxCpltCallback(_UNUSED_ I2C_HandleTypeDef *hi2c)
{
	BEGIN_ISR
	BaseType_t higher=0;
	xTaskNotifyFromISR(ina3221_taskHandle, NOTIF_INA_RDCOMPL, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
	END_ISR;
}



void HAL_I2C_ErrorCallback(_UNUSED_ I2C_HandleTypeDef *hi2c)
{
	BEGIN_ISR
	lastErr = hi2c->ErrorCode;
	BaseType_t higher=0;
	xTaskNotifyFromISR(ina3221_taskHandle, NOTIF_INA_ERR, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
	END_ISR;
}




#else // INA3221_TASK ==0 : process in IT

// ----------------------------------------------------------------------------------


//#include "railconfig.h" // for ugly hack

 int disable_ina3221 = 0; // global disable (when debugging something else)

extern uint32_t GetCurrentMicro(void);

volatile int ina_used = 0;

_UNUSED_ static int ina3221_write16dma(int a, int reg, uint16_t v)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
	if ((1)) w16 = __builtin_bswap16(v);
	else w16=v;
	//__HAL_DMA_ENABLE_IT(&hdma_i2c3_tx, DMA_IT_TC);
	if (HAL_I2C_GetState(&INA3221_I2C_PORT) != HAL_I2C_STATE_READY) {
    	bkpoint(4, INA3221_I2C_PORT.ErrorCode);
    	return -1;
	}
    status = HAL_I2C_Mem_Write_DMA(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2);
    if (status != HAL_OK) {
    	itm_debug1(DBG_INA3221|DBG_ERR, "i2c wd err", status);
       	ina3221_errors++;
    	bkpoint(5, INA3221_I2C_PORT.ErrorCode);
    	return -1;
    }
    return 0;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------

/*
 * WARNING : ina3221_trigger_conversion is called
 * from timer interrupt !
 */

#ifndef INA3221_USE_DMA
#ifndef INA3221_USE_IT
#define INA3221_USE_POLL
#endif
#endif

static int ina3221_init_done = 0;

#ifdef INA3221_USE_POLL
void ina3221_trigger_conversion(void)
{
	if (disable_ina3221) return;
	uint32_t t0trig = GetCurrentMicro();
	if (HAL_I2C_GetState(&INA3221_I2C_PORT) != HAL_I2C_STATE_READY) {
    	bkpoint(6, INA3221_I2C_PORT.ErrorCode);
		return;
	}
	for (int dev = 0; dev<3; dev++) {
		if (!ina3221_devices[dev]) continue;
		int addr = 0x40 + dev;
		itm_debug1(DBG_INA3221, "ina trig", dev);
#if INA3221_USE_DMA
		//__HAL_DMA_ENABLE_IT(&hdma_i2c3_tx, DMA_IT_TC);
		ina3221_write16dma(addr, INA3221_REG_CONFIG, ina_conf_val);
#else
		ina3221_write16(addr, INA3221_REG_CONFIG, ina_conf_val);
#endif
		itm_debug1(DBG_INA3221, "ina/Tr", addr);
	}
	uint32_t ttrig = GetCurrentMicro() - t0trig;
	itm_debug2(DBG_INA3221, "ina trg", ttrig, ina3221_errors);
}
#else

static int trigdev = -1;
static uint32_t t0trig =0;

static int _nextdev(int dev)
{
	dev = dev+1;
	for (;dev<3;dev++) {
		if (ina3221_devices[dev]) return dev;
	}
	return -1;
}

static void _trig_complete(int);

static void _trig_conv(int dev)
{
	itm_debug1(DBG_INA3221, "_trig_conv", GetCurrentMicro());
	if (HAL_I2C_GetState(&INA3221_I2C_PORT) != HAL_I2C_STATE_READY) {
    	bkpoint(7, INA3221_I2C_PORT.ErrorCode);
    	// 0x21 busy tx
    	_trig_complete(1);
		return;
	}
	trigdev = _nextdev(dev);

	itm_debug2(DBG_INA3221, "ina trig", dev, GetCurrentMicro());
	int addr = 0x40 + dev;
	int rc;
#if INA3221_USE_DMA
	//__HAL_DMA_ENABLE_IT(&hdma_i2c3_tx, DMA_IT_TC);
	rc = ina3221_write16dma(addr, INA3221_REG_CONFIG, ina_conf_val);
#else
	rc = ina3221_write16it(addr, INA3221_REG_CONFIG, ina_conf_val);
#endif
	itm_debug1(DBG_INA3221, "ina/Tr", addr);
	if (rc) {
		_trig_complete(1);
	}
}

static void _trig_complete(int err)
{
	trigdev = -1;
	__sync_fetch_and_sub(&ina_used, 1);
	uint32_t ttrig = GetCurrentMicro() - t0trig;
	itm_debug3(DBG_ERR|DBG_INA3221, "end trg", err, ttrig, ina3221_errors);
}

void ina3221_trigger_conversion(void)
{
	if (disable_ina3221) return;
	if (!ina3221_init_done) return;
	uint8_t t = __sync_fetch_and_add(&ina_used, 1);
	if (t) {
		bkpoint(201, ina_used);
		__sync_fetch_and_sub(&ina_used, 1);
		return;
	}
	t0trig = GetCurrentMicro();

	if (trigdev != -1) {
    	bkpoint(100, INA3221_I2C_PORT.State);
		__sync_fetch_and_sub(&ina_used, 1);
    	return;
	}
	trigdev = _nextdev(-1);
	if (trigdev<0) {
		_trig_complete(0);
		return;
	}
	_trig_conv(trigdev);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	BEGIN_ISR
	itm_debug1(DBG_INA3221, "txcpl", 0);
	if (trigdev<0) _trig_complete(0);
	else {
		_trig_conv(trigdev);
	}
	END_ISR;
}
#endif

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
	uint8_t t = __sync_fetch_and_add(&ina_used, 1);
	if (t) {
		bkpoint(200, ina_used);
		__sync_fetch_and_sub(&ina_used, 1);
		return;
	}
	pflagdone = flagdone;
	if (pflagdone) *pflagdone = 0;


	pvalues = vals;

	itm_debug1(DBG_INA3221, "ina rd", ina3221_errors);
	//t0 = HAL_GetTick();
	t0read = GetCurrentMicro();
	ina3221_inter_dur = t0read - t1;
	itm_debug1(DBG_INA3221, "ina inter", ina3221_inter_dur);
	get_reg_step = 0;
	_get_next_reg();
}

static void _end_next_reg(int err)
{
	if (pflagdone) *pflagdone = 1;
	uint32_t tm = GetCurrentMicro();
	ina3221_scan_dur = tm - t0read;
	t1 = tm;
	get_reg_step = -1;
	itm_debug2(DBG_ERR|DBG_INA3221,"ina done", ina3221_scan_dur, tm);
	__sync_fetch_and_sub(&ina_used, 1);
}
static void _get_next_reg(void)
{
	itm_debug1(DBG_INA3221, "get_next_reg", get_reg_step);
	for (;;) {
		if (get_reg_step<0) {
			itm_debug1(DBG_INA3221|DBG_ERR, "bad get_reg_step", get_reg_step);
		}
		if (get_reg_step == INA3221_NUM_VALS) {
			_end_next_reg(0);
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
		itm_debug1(DBG_INA3221, "gns read", get_reg_step);
		HAL_StatusTypeDef status;
		int addr = 0x40 + numdev;
		if (__HAL_I2C_GET_FLAG(&INA3221_I2C_PORT, I2C_FLAG_BUSY) != RESET) {
			itm_debug1(DBG_ERR|DBG_INA3221, "busy", 0);
			_end_next_reg(1);
			return;
		}
#if INA3221_USE_DMA
		//__HAL_DMA_ENABLE_IT(&hdma_i2c3_rx, DMA_IT_TC);
		status = HAL_I2C_Mem_Read_DMA(&INA3221_I2C_PORT, addr<<1, reg, I2C_MEMADD_SIZE_8BIT,
	    		(uint8_t *)&pvalues[get_reg_step], 2);
#else
		status = HAL_I2C_Mem_Read_IT_NW(&INA3221_I2C_PORT, addr<<1, reg, I2C_MEMADD_SIZE_8BIT,
	    		(uint8_t *)&pvalues[get_reg_step], 2);
#endif
		if (status != HAL_OK) {
			itm_debug1(DBG_INA3221|DBG_ERR, "readit", status);
			_err();
			_end_next_reg(1);
			return;
		}
		get_reg_step++;
		break;
	}
}


void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	BEGIN_ISR
	if (get_reg_step==3) {
		itm_debug1(DBG_ERR, "inar2", pvalues[2]);
	}
	_get_next_reg();
	END_ISR;
}



void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	BEGIN_ISR
	// XXX TODO check this is for us
	itm_debug2(DBG_INA3221|DBG_ERR, "i2c err", hi2c->ErrorCode, get_reg_step);
	ina3221_errors++;
	bkpoint(10, hi2c->ErrorCode);
	//get_reg_step = -1;
	if ((0)) disable_ina3221 = 1;
	if ((0)) _err();
	if (ina_used) {
		if (trigdev != -1) {
			// error while triggering
			// abort trigger
			_trig_complete(1);
		} else if (get_reg_step != -1) {
			// try to continue
			_get_next_reg();
		} else {
			bkpoint(1000, hi2c->ErrorCode);
		}
	} else {
		bkpoint(1001, hi2c->ErrorCode);
	}
	END_ISR;
}
void HAL_I2C_MasterRxCpltCallback (I2C_HandleTypeDef * hi2c)
{
	BEGIN_ISR
	itm_debug1(DBG_INA3221, "i2c rx cpl",0);
  // RX Done .. Do Something!
	END_ISR;
}
/*
void HAL_I2C_MasterTxCpltCallback (I2C_HandleTypeDef * hi2c)
{
	bkpoint();
	itm_debug1(DBG_INA3221, "txcpli", 0);
	if (trigdev<0) _trig_complete();
	else {
		_trig_conv(trigdev);
	}
  // TX Done .. Do Something!
}


*/
#endif // INA3221_TASK

// ----------------------------------------------------------------------------------


static uint16_t ina3221_read16(int a, int reg)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
    status = HAL_I2C_Mem_Read(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) {
    	itm_debug1(DBG_INA3221|DBG_ERR, "i2c r err", status);
    	ina3221_errors++;
    	bkpoint(1, INA3221_I2C_PORT.ErrorCode);
    	return 0;
    } else {
    	return __builtin_bswap16(w16);
    }
}

static int ina3221_write16(int a, int reg, uint16_t v)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
	if ((1)) w16 = __builtin_bswap16(v);
	else w16=v;
    status = HAL_I2C_Mem_Write(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) {
    	itm_debug1(DBG_INA3221|DBG_ERR, "i2c w err", status);
       	ina3221_errors++;
    	bkpoint(2, INA3221_I2C_PORT.ErrorCode);
    	return -1;
    }
    return 0;
}




_UNUSED_ static int ina3221_write16it(int a, int reg, uint16_t v)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
	if ((1)) w16 = __builtin_bswap16(v);
	else w16=v;

	if (HAL_I2C_GetState(&INA3221_I2C_PORT) != HAL_I2C_STATE_READY) {
    	bkpoint(3, INA3221_I2C_PORT.State);
    	return -1;
	}
#if 0
    status = HAL_I2C_Mem_Write_IT(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2);
#else
    status = HAL_I2C_Mem_Write_IT_NW(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2);
#endif
    if (status != HAL_OK) {
    	itm_debug1(DBG_INA3221|DBG_ERR, "i2c w err", status);
       	ina3221_errors++;
    	bkpoint(3, INA3221_I2C_PORT.ErrorCode);
    	itm_debug1(DBG_INA3221|DBG_ERR, "i2c rst", lastErr);
    	HAL_I2C_Init(&INA3221_I2C_PORT);
    	return -1;
    }
    return 0;
}


/* Private define for @ref PreviousState usage */
#define I2C_STATE_MSK             ((uint32_t)((uint32_t)((uint32_t)HAL_I2C_STATE_BUSY_TX | (uint32_t)HAL_I2C_STATE_BUSY_RX) & (uint32_t)(~((uint32_t)HAL_I2C_STATE_READY)))) /*!< Mask State define, keep only RX and TX bits            */
#define I2C_STATE_NONE            ((uint32_t)(HAL_I2C_MODE_NONE))                                                        /*!< Default Value                                          */
#define I2C_STATE_MASTER_BUSY_TX  ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MASTER))            /*!< Master Busy TX, combinaison of State LSB and Mode enum */
#define I2C_STATE_MASTER_BUSY_RX  ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MASTER))            /*!< Master Busy RX, combinaison of State LSB and Mode enum */
#define I2C_STATE_SLAVE_BUSY_TX   ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_SLAVE))             /*!< Slave Busy TX, combinaison of State LSB and Mode enum  */
#define I2C_STATE_SLAVE_BUSY_RX   ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_SLAVE))             /*!< Slave Busy RX, combinaison of State LSB and Mode enum  */

#define I2C_TIMEOUT_FLAG          35U         /*!< Timeout 35 ms             */
#define I2C_TIMEOUT_BUSY_FLAG     25U         /*!< Timeout 25 ms             */
#define I2C_TIMEOUT_STOP_FLAG     5U          /*!< Timeout 5 ms              */
#define I2C_NO_OPTION_FRAME       0xFFFF0000U /*!< XferOptions default value */

HAL_StatusTypeDef HAL_I2C_Mem_Write_IT_NW(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
  __IO uint32_t count = 0U;

  /* Check the parameters */
  assert_param(IS_I2C_MEMADD_SIZE(MemAddSize));

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    /* Wait until BUSY flag is reset */
    count = 2; //I2C_TIMEOUT_BUSY_FLAG * (SystemCoreClock / 25U / 1000U);
    do
    {
      count--;
      if (count == 0U)
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
    while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) != RESET);

    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Check if the I2C is already enabled */
    if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
    {
      /* Enable I2C peripheral */
      __HAL_I2C_ENABLE(hi2c);
    }

    /* Disable Pos */
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

    hi2c->State     = HAL_I2C_STATE_BUSY_TX;
    hi2c->Mode      = HAL_I2C_MODE_MEM;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr    = pData;
    hi2c->XferCount   = Size;
    hi2c->XferSize    = hi2c->XferCount;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;
    hi2c->Devaddress  = DevAddress;
    hi2c->Memaddress  = MemAddress;
    hi2c->MemaddSize  = MemAddSize;
    hi2c->EventCount  = 0U;

    /* Generate Start */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    /* Note : The I2C interrupts must be enabled after unlocking current process
    to avoid the risk of I2C interrupt handle execution before current
    process unlock */

    /* Enable EVT, BUF and ERR interrupt */
    __HAL_I2C_ENABLE_IT(hi2c, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  Read an amount of data in non-blocking mode with Interrupt from a specific memory address
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  DevAddress Target device address
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT_NW(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{
  __IO uint32_t count = 0U;

  /* Check the parameters */
  assert_param(IS_I2C_MEMADD_SIZE(MemAddSize));

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    /* Wait until BUSY flag is reset */
    count = 2;//I2C_TIMEOUT_BUSY_FLAG * (SystemCoreClock / 25U / 1000U);
    do
    {
      count--;
      if (count == 0U)
      {
        hi2c->PreviousState       = I2C_STATE_NONE;
        hi2c->State               = HAL_I2C_STATE_READY;
        hi2c->Mode                = HAL_I2C_MODE_NONE;
        hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
    while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) != RESET);

    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Check if the I2C is already enabled */
    if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
    {
      /* Enable I2C peripheral */
      __HAL_I2C_ENABLE(hi2c);
    }

    /* Disable Pos */
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);

    hi2c->State     = HAL_I2C_STATE_BUSY_RX;
    hi2c->Mode      = HAL_I2C_MODE_MEM;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr    = pData;
    hi2c->XferCount   = Size;
    hi2c->XferSize    = hi2c->XferCount;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;
    hi2c->Devaddress  = DevAddress;
    hi2c->Memaddress  = MemAddress;
    hi2c->MemaddSize  = MemAddSize;
    hi2c->EventCount  = 0U;

    /* Enable Acknowledge */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);

    /* Generate Start */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    if (hi2c->XferSize > 0U)
    {
      /* Note : The I2C interrupts must be enabled after unlocking current process
      to avoid the risk of I2C interrupt handle execution before current
      process unlock */

      /* Enable EVT, BUF and ERR interrupt */
      __HAL_I2C_ENABLE_IT(hi2c, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);
    }
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

static void _ina3221_configure(int a, int continuous)
{
	//HAL_StatusTypeDef status;
	uint16_t w16;

    _UNUSED_ uint16_t mid = ina3221_read16(a, INA3221_REG_MANUFACTURER_ID);
     // 0x5449
    _UNUSED_ uint16_t did = ina3221_read16(a,  INA3221_REG_DIE_ID);
    // 0x3220

    _UNUSED_ uint16_t cnfbr = ina3221_read16(a,  INA3221_REG_CONFIG);

    if ((1)) {
    	w16 = INA3221_CONF_RESET;
    	int rc = ina3221_write16(a, INA3221_REG_CONFIG, w16);
    	if (rc) {
    		bkpoint(100,rc);
    	}
    	osDelay(1000);
    	//if ((1)) return;
    }
    _UNUSED_ uint16_t cnfar = ina3221_read16(a,  INA3221_REG_CONFIG);

    w16 = INA3221_CONF_CH1_EN | INA3221_CONF_CH2_EN | INA3221_CONF_CH3_EN
    		| INA3221_CONF_VS_CT_140u
			| INA3221_CONF_MODE_SHUNT;
    w16 |= (continuous ? INA3221_CONF_AVG16 : INA3221_CONF_AVG1);

    if (continuous) w16 |= INA3221_CONF_MODE_CONTINUOUS;
	ina_conf_val = w16;
	int rc = ina3221_write16(a, INA3221_REG_CONFIG, w16);
    if ((0)) osDelay(100*1);
    if (rc) bkpoint(101, rc);
    rc = ina3221_write16(a, INA3221_REG_MASK_ENABLE, 0);
    if (rc) bkpoint(102, rc);

    _UNUSED_ uint16_t cnfac = ina3221_read16(a,  INA3221_REG_CONFIG);
    //if ((0)) ina3221_start_read();

    return;
}

void _ina3221_init(int continuous)
{
	if (disable_ina3221) return;
	//I2C_Scan();
	for (int dev = 0; dev<4; dev++) {
		int addr = 0x40 + dev;
	    HAL_StatusTypeDef res;
        res = HAL_I2C_IsDeviceReady(&INA3221_I2C_PORT, addr << 1, 1, 10);
        if (res == HAL_OK) {
        	_ina3221_configure(addr, continuous);
        	itm_debug2(DBG_PRES|DBG_INA3221, "INA@", dev, addr);
        	ina3221_devices[dev]=1;
        } else {
        	ina3221_devices[dev]=0;
        }
	}
	ina3221_init_done = 1;
}

static void ina3221_init_and_configure(void)
{
	_ina3221_init(INA3221_CONTIUNOUS);
}

// ----------------------------------------------------------------------------------

void ina3221_task_start(_UNUSED_ void *argument)
{
#if INA3221_TASK
	if (DISABLE_INA3221) {
		for(;;) {
			osDelay(1);
		}
	}
	run_ina_task();
#else
	for(;;) {
		osDelay(1);
	}
#endif
}


