/*
 * ina3221.c
 *
 *  Created on: Apr 13, 2021
 *      Author: danielbraun
 */

#include "../../misc.h"





#if defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"

#else
#error no board hal
#endif


#include "cmsis_os.h"

#include "ina3221.h"
#include "ina3221_def.h"
#include "ina3221_config.h"
//#include "utils/microsec.h"

#include "../../msg/trainmsg.h"
#include "../../oscillo/oscillo.h"
#include "../../statval.h"

#include "../../ctrl_detect/train_detectors_params.h"


#ifndef BOARD_HAS_INA3221
#error BOARD_HAS_INA3221 not defined, remove this file from build
#endif


// ----------------------------------------------------------------------------------
// global run mode, each tasklet implement this
static runmode_t run_mode = 0;
static uint8_t testerAddr;

// ----------------------------------------------------------------------------------


uint8_t ina3221_devices[4] = {0, 0, 0, 0};  // 1 if device is present

uint16_t ina3221_errors = 0;
static uint8_t disable_ina3221 = 0;
static int ina3221_init_done = 0;
static int ina_conf_val = 0;

static int16_t ina_svalues[INA3221_NUM_VALS] = {0}; // byteswapped values




const stat_val_t statval_ina3221[] = {
	    { ina_svalues, 0, 2       _P("ina0")},
	    { ina_svalues, 2, 2       _P("ina1")},
	    { ina_svalues, 4, 2       _P("ina2")},
	    { NULL, 0, 0 _P(NULL)}
};

#ifndef INA3221_TASKRD
#error hu?
#endif



static void bkpoint(int loc, int err)
{
	itm_debug2(DBG_ERR|DBG_INA3221, "INA ERR", loc, err);
#ifdef BOARD_HAS_OSCILLO
	extern volatile int oscillo_trigger_start;
	oscillo_trigger_start = 200;
#endif
}


static int ina3221_write16(int a, int reg, uint16_t v);
static uint16_t ina3221_read16(int a, int reg);

static void ina3221_init_and_configure(void);
static void start_detectfreq_read(void);

// ----------------------------------------------------------------------------------




extern osThreadId ina3221_taskHandle;
static int lastErr = 0;
//static int cvrf_dev = 0;

typedef enum {
	state_idle,

	state_trig_0,
	state_trig_1,
	state_trig_2,
	state_trig_3,

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

//static int _trig(int dev);
static void _reg_read(int dev, int reg);
static void _read_complete(int err);
static int _next_dev(int d);


static void handle_ina_notif_normal(uint32_t notif);
static void handle_ina_notif_detectfreq(uint32_t notif);
static void run_ina_task(void);

//static volatile uint16_t mask_en_val = 0;
static 	ina_state_t state = state_idle;

volatile int16_t oscillo_ina0;
volatile int16_t oscillo_ina1;
volatile int16_t oscillo_ina2;

static uint16_t detect2_bitfield = 0;
static uint16_t detect2_mode = 0;

static int8_t presence[INA3221_NUM_VALS] = {0};



static int singledev = -1;
static int singlech = -1;


void ina3221_task_start(_UNUSED_ void *argument)
{
	if (DISABLE_INA3221) {
		for(;;) {
			osDelay(1);
		}
	}
	run_ina_task();
	itm_debug1(DBG_ERR, "impossible", 0);
}

void brklong(int32_t l)
{
	itm_debug1(DBG_ERR, "long notif", l);
}



static void run_ina_task(void)
{
	ina3221_init_and_configure(); //XXX
	_UNUSED_ int nstuck = 0;
	for (;;) {
		uint32_t notif = 0;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		if ((0)) itm_debug1(DBG_INA3221|DBG_DETECT, "ina---", notif);
		uint32_t t0 = HAL_GetTick();

		switch(run_mode) {
		default: // FALLTHRU
		case runmode_normal:
			handle_ina_notif_normal(notif);
			break;
		case runmode_detect2:
			switch (detect2_mode) {
			default: // FALLTHRY
			case 1:
				handle_ina_notif_normal(notif);
				break;
			case 2:
				handle_ina_notif_detectfreq(notif);
				break;
			}
		}
		uint32_t t1 = HAL_GetTick();
		if ((t1-t0) > 3) {
			brklong(t1-t0);
		}
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
					if ((m.v1u == runmode_normal) && (run_mode==runmode_detect2)) {
						itm_debug1(DBG_DETECT, "brk here", 0);
						//p -elements unlimited -- timeval
					}
					run_mode = m.v1u;
					itm_debug1(DBG_INA3221|DBG_DETECT, "INA:mode", run_mode);
					testerAddr = m.from;
					memset(presence, 0, sizeof(presence));
					// reconf on mode change
					ina3221_init_and_configure();
					itm_debug1(DBG_INA3221|DBG_DETECT, "INA:cnfok", run_mode);
				}
				continue;
				break;
			case CMD_START_INA_MONITOR:
				if (run_mode != runmode_detect2) {
					itm_debug1(DBG_ERR|DBG_INA3221, "bad rmode", run_mode);
					continue;
				}
				memset(presence, 0, sizeof(presence));
				itm_debug2(DBG_DETECT|DBG_INA3221, "detect2 ina", m.va16, m.vb8);
				detect2_bitfield = m.va16;
				detect2_mode = m.vb8;
				if (detect2_mode == 2) {
					// frequency mode
					start_detectfreq_read();
				}
				continue;
			default:
				//itm_debug2(DBG_INA3221|DBG_DETECT, "unk msg", m.cmd, run_mode);
				break;
			}
		}
	}
}

static void handle_ina_notif_normal(uint32_t notif)
{
	//int rc;
	if (notif & NOTIF_INA_TRIG) {
		// just ignore (INA3221_CONTIUNOUS)
	}
	if (notif & NOTIF_INA_WRCOMPL) {
		/*
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
				state = state_idle;
			}
		} else {
			bkpoint(1,1000);
		}
		*/
	}
	if (notif & NOTIF_INA_READ) {
		/*
		 * NOTIF_INA_READ is set by TIM1 interrupt
		 * it triggers reading all INA devices (and all channels on each device)
		 */
		if ((1)) itm_debug1(DBG_INA3221, "INA:N:RD", notif);
		int dev = _next_dev(-1);
		if (dev >= 0) {
			state = state_rd_0 + dev * 3;
			_reg_read(dev, 0);
		} else {
			_read_complete(0);
			state = state_idle;
		}
	}
	if (notif & NOTIF_INA_RDCOMPL) {
		itm_debug1(DBG_INA3221, "INA:N:CPL", state);

		if ((state >= state_rd_0) && (state <= state_rd_11)) {
			int reg = (state - state_rd_0) % 3;
			int dev = (state - state_rd_0) / 3;
			if (reg==2){
				dev = _next_dev(dev);
				if (dev >= 0) {
					// state = (state_rd_0 + dev) * 3; oups !!!
					state = state_rd_0 + dev * 3;
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



// ---------------------------------------------------------------------------

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
    	osDelay(50);
    	//if ((1)) return;
    }
    _UNUSED_ uint16_t cnfar = ina3221_read16(a,  INA3221_REG_CONFIG);


    const int usealrm = 0;
    // by setting MODE_SHUNT and configuring CH1_CRIT_LIM, we can
    // use the "critical" led, which will twinckle e.g. to identify physical module

    if ((!usealrm)) {
    	w16 = INA3221_CONF_CH1_EN | INA3221_CONF_CH2_EN | INA3221_CONF_CH3_EN
    		| INA3221_CONF_VS_CT_140u
			| INA3221_CONF_MODE_SHUNT;
    } else {
    	w16 = INA3221_CONF_CH1_EN | INA3221_CONF_CH2_EN | INA3221_CONF_CH3_EN
    		| INA3221_CONF_VS_CT_140u
			| INA3221_CONF_MODE_SHUNT
			| INA3221_CONF_MODE_BUSV
			;
    }
    w16 |= (continuous ? INA3221_CONF_AVG16 : INA3221_CONF_AVG1);

    if (continuous) w16 |= INA3221_CONF_MODE_CONTINUOUS;
	ina_conf_val = w16;
	int rc = ina3221_write16(a, INA3221_REG_CONFIG, w16);
    if ((0)) osDelay(100*1);

    if ((usealrm)) {
    	ina3221_write16(a, INA3221_REG_CH1_CRIT_LIM, 1);
    }

    if (rc) bkpoint(101, rc);
    rc = ina3221_write16(a, INA3221_REG_MASK_ENABLE, 0);
    if (rc) bkpoint(102, rc);

    _UNUSED_ uint16_t cnfac = ina3221_read16(a,  INA3221_REG_CONFIG);


}

void _ina3221_init(int continuous)
{
	if (disable_ina3221) return;
	singledev = -1;
	singlech = -1;
	//I2C_Scan();
	for (int dev = 0; dev<4; dev++) {
		int addr = 0x40 + dev;
	    HAL_StatusTypeDef res;
        res = HAL_I2C_IsDeviceReady(&INA3221_I2C_PORT, addr << 1, 1, 10);
        if (res == HAL_OK) {

        	_ina3221_configure(addr, continuous);

        	itm_debug2(DBG_PRES|DBG_INA3221, "INA@", dev, addr); // 0x40 0x43
        	ina3221_devices[dev]=1;
        } else {
        	ina3221_devices[dev]=0;
        }
	}
	ina3221_init_done = 1;

	/*if ((0)) {
		for (;;) {
			for (int dev=0; dev<4; dev++) {
				if (!ina3221_devices[dev]) continue;
				osDelay(500);
				int addr = 0x40 + dev;
				uint16_t me = ina3221_read16(addr,  INA3221_REG_MASK_ENABLE);
				itm_debug2(DBG_INA3221, "me", dev, me);
			}
		}
	}*/
}

static void ina3221_init_and_configure(void)
{
	_ina3221_init(INA3221_CONTIUNOUS);
}

// ----------------------------------------------------------------------------------


static uint16_t ina_uvalues[INA3221_NUM_VALS] = {0}; // raw values

static int _next_dev(int dev)
{
	if (singledev != -1) {
		return singledev;
	}
	dev = dev+1;
	for (;dev<=3;dev++) {
		if (ina3221_devices[dev]) return dev;
	}
	return -1;
}

/*
static int  _trig(int dev)
{
	if (disable_ina3221) return 0;
	cvrf_dev = dev;
	int addr = 0x40 + dev;
	itm_debug2(DBG_INA3221, "TRIG", dev, addr);
	return ina3221_write16it(addr, INA3221_REG_CONFIG, ina_conf_val);
}
*/

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
		itm_debug2(DBG_ERR|DBG_INA3221, "busy", dev, nreg);
		// TODO _end_next_reg(1);
		/*
		 * if this happens systematically, including on first read : check that i2c IRQ
		 * are enabled !
		 */
		return;
	}

	status = HAL_I2C_Mem_Read_IT(&INA3221_I2C_PORT, addr<<1, hwreg, I2C_MEMADD_SIZE_8BIT,
			(uint8_t *)&ina_uvalues[dev*3+nreg], 2);
	if (status != HAL_OK) {
		itm_debug1(DBG_INA3221|DBG_ERR, "readit", status);
		// TODO
		return;
	}
}



static void _read_complete(_UNUSED_ int err)
{
	itm_debug1(DBG_INA3221, "rd:cpl", 0);
	for (int i = 0; i<INA3221_NUM_VALS; i++) {
		ina_svalues[i] = (int16_t) __builtin_bswap16(ina_uvalues[i]);
	}
	msg_64_t m = {0};

	oscillo_ina0 = ina_svalues[0];
	oscillo_ina1 = ina_svalues[1];
	oscillo_ina2 = ina_svalues[2];

	switch (run_mode) {
	default:
		break;
	case runmode_detect_experiment:
		itm_debug1(DBG_INA3221, "D/1", ina_svalues[1]);
		m.from =  MA0_INA(oam_localBoardNum());
		m.to = MA3_UI_GEN;
		m.cmd = CMD_INA3221_VAL1;
		m.subc = 1;
		m.v1 = ina_svalues[1];
		mqf_write_from_ina3221(&m);
		break;


	case runmode_detect2:
		//itm_debug1(DBG_DETECT, "rdcplD", detect2_monitor);
		switch (detect2_mode) {
		case 1:
			/* normal ina3221 presence detection, applied to detection */
			for (int i = 0; i<INA3221_NUM_VALS; i++) {
				if (0 == (detect2_bitfield & (1<<i))) continue;
				int p = (abs(ina_svalues[i])>1000) ? 1 : 0;
				if (p == presence[i]) continue;
				presence[i] = p;
				if (!p) continue;

				m.from = MA0_INA(oam_localBoardNum());
				m.to = MA1_CONTROL();
				m.cmd = CMD_DETECTION_REPORT;
				m.subc = i;
				m.v1 = ina_svalues[i];
				if ((0)) itm_debug2(DBG_DETECT, "report", i, ina_svalues[i]);
				mqf_write_from_ina3221(&m);
			}
			break;

		case 0:
			// ignore
			break;

		default:
			itm_debug2(DBG_ERR|DBG_DETECT|DBG_INA3221, "bad dmode", detect2_mode, detect2_bitfield);
			break;
		}
		break;


	case runmode_normal:
		for (int i = 0; i<INA3221_NUM_VALS; i++) {
			if ((i<=2)) itm_debug2(DBG_INA3221, "ina val", i, ina_svalues[i]);
			int p = (abs(ina_svalues[i])>1000) ? 1 : 0;
			if (p == presence[i]) continue;
			presence[i] = p;
			itm_debug3(DBG_INA3221|DBG_PRES, "PRSCH", i,p, ina_svalues[i]);
			// notify change
			m.from = MA0_INA(oam_localBoardNum()); // change to MA_INA3221_B ?
			m.to = MA1_CONTROL();
			m.cmd = CMD_PRESENCE_SUB_CHANGE;
			m.subc = i;
			m.v1u = p;
			m.v2 = ina_svalues[i];
			mqf_write_from_ina3221(&m);


			static int cnt=0;
			cnt++;
			if ((0) && ((cnt%50)==0)) {
				msg_64_t m;
				static int16_t v[12];
				memcpy(v, ina_svalues, 12*2);
				for (int i=0; i<12; i++) {
					if (abs(v[i]) > 400) {
						itm_debug2(DBG_INA3221, "ina big", i, v[i]);
					}
				}
				if ((1)) v[7] = cnt;
				m.from = MA0_INA(oam_localBoardNum());
				m.to = MA3_UI_GEN;
				m.cmd = CMD_INA3221_UI_REPORT;
				m.v32u = (uint32_t) v;
				mqf_write_from_ina3221(&m);
			}
			break;
		}
	}
}

// ---------------------------------------------------


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


// ----------------------------------------------------------------------------------

#define INA_I2C_TIMEOUT  100

static uint16_t ina3221_read16(int a, int reg)
{
	HAL_StatusTypeDef status;
	uint16_t w16;
    status = HAL_I2C_Mem_Read(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2, INA_I2C_TIMEOUT);
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
    status = HAL_I2C_Mem_Write(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2, INA_I2C_TIMEOUT);
    if (status != HAL_OK) {
    	itm_debug1(DBG_INA3221|DBG_ERR, "i2c w err", status);
       	ina3221_errors++;
    	bkpoint(2, INA3221_I2C_PORT.ErrorCode);
    	return -1;
    }
    return 0;
}


// --------------------------------------------------------------------

static uint8_t df_dev=0;
static uint8_t df_reg = 0;
static uint32_t df_t0 = 0;
static uint16_t df_idx = 0;

typedef struct {
	uint32_t tick;
	int16_t val;
} time_val_t;

#define DF_NUMVAL (1024*2)
static time_val_t timeval[DF_NUMVAL] = {0};


typedef struct freq_avg {
	int32_t sum;
	uint16_t n;
} freq_avg_t;

static freq_avg_t freqavg[FREQ_NSTEPS];
static int freq_st;
static int freq_idx;
static uint32_t freq_stoptick;
static uint32_t freq_starttick;

static void start_detectfreq_read(void)
{
	// mode 2 detection, detect2_bitfield is actually ina num
	df_dev = detect2_bitfield / 3;
	df_reg = detect2_bitfield % 3;
	itm_debug3(DBG_DETECT, "inafreq", detect2_bitfield, df_dev, df_reg);
	df_t0 = HAL_GetTick();
	df_idx = 0;
	memset(timeval, 0, sizeof(timeval));

	memset(freqavg, 0, sizeof(freqavg));
	freq_st = 0;
	freq_idx = 0;
	freq_stoptick = 0;
	freq_starttick = 0;

	_reg_read(df_dev, df_reg);

}

static void df_complete(void);

static void handle_ina_notif_detectfreq(uint32_t notif)
{
	if (notif & NOTIF_INA_TRIG) {
	}
	if (notif & NOTIF_INA_WRCOMPL) {
	}
	if (notif & NOTIF_INA_READ) {
	}
	if (notif & NOTIF_INA_RDCOMPL) {
		uint32_t tick = HAL_GetTick();
		tick = tick - df_t0;
		//itm_debug3(DBG_INA3221|DBG_DETECT, "INA:N:CPL", tick, freq_st, freq_idx);
		int16_t val = (int16_t) __builtin_bswap16(ina_uvalues[df_dev*3+df_reg]);
		//itm_debug3(DBG_INA3221|DBG_DETECT, "rdcmpl", df_dev, df_reg, val);

		if (val<0) val = -val;

		switch (freq_st) {
		default:
		case -1:
			break;

		case 0:
			//waiting for begin
			if (val > 50) {
				freq_starttick = 0;
				freq_stoptick = tick+FREQ_FIRST_STEP_DUR-4;
				itm_debug2(DBG_DETECT, "xxx st", 1, 0);
				freq_st = 1;
				freq_idx = 0;
			}
			break;
		case 1:
			if (tick >= freq_stoptick) {
				freq_idx++;
				if (freq_idx >= FREQ_NSTEPS) {
					itm_debug1(DBG_DETECT, "xxx done",0);
					freq_st = -1;
					df_complete();
					break;
				}
				itm_debug2(DBG_DETECT, "xxx st", 2, freq_idx);
				freq_st = 2;
				freq_starttick = tick+FREQ_RELAX_TIME;
			} else {
				freqavg[freq_idx].sum += val;
				freqavg[freq_idx].n++;
			}
			break;
		case 2:
			if (tick >= freq_starttick) {
				freq_st = 1;
				freq_stoptick = tick+FREQ_STEP_DUR;
				itm_debug2(DBG_DETECT, "xxx st", 1, freq_idx);
			}
			break;
		}

		if (freq_st>=0) {
			_reg_read(df_dev, df_reg);
		}
		//itm_debug3(DBG_DETECT, "inardf:", df_dev, df_reg, val /*ina_uvalues[df_dev*3+df_reg]*/);
		//itm_debug2(DBG_DETECT, "inardf:",df_idx, val );

		if (df_idx < DF_NUMVAL) {
			timeval[df_idx].tick =  (uint16_t) tick;
			timeval[df_idx].val = val;
			df_idx++;

			if ((0)) { // debug/test, read all INAs regs
				static int c=0;
				df_dev = (c%12)/3;
				df_reg = c%3;
				c++;
			}

		} else {
			//df_complete();
		}
		if (df_idx==510) {
			itm_debug1(DBG_DETECT, "break here", 0);
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


static void df_complete(void)
{
	int32_t v0 = 0;
	for (int i=0; i<FREQ_NSTEPS; i++) {
		int32_t k = 0;
		int32_t val = 0;
		if (freqavg[i].n) {
			if (!i) v0 = freqavg[i].sum/freqavg[i].n;
			val = freqavg[i].sum/freqavg[i].n;
			if (i) {
				if (v0) k = (100*val)/v0;
			} else {
				k = 100;
			}
		}
		itm_debug3(DBG_DETECT, "inafreq", i, val, k);
	}
	for (int i=0; i<DF_NUMVAL; i++) {
		if (i && !timeval[i].tick) break;
		itm_debug3(DBG_DETECT, "inard", i, timeval[i].tick, timeval[i].val);
	}
	itm_debug1(DBG_DETECT, "done inard", DF_NUMVAL);
}

typedef enum {
	loco_unknown = 0,
	Marklin8805_BR29,
	Marklin8821_V200,
} locomotive_t;

typedef struct {
	locomotive_t loco;
	uint16_t kval_t[8];
};

static const kval_t kvals[] =
{ Marklin8805_BR29, 856,   75, 57, 41, 14,  5,  2,  4},
{ Marklin8805_BR29, 633,   77, 68, 50, 26, 10,  4,  6},
{ Marklin8805_BR29, 775,   70, 64, 45, 12,  2,  1,  2},
{ Marklin8805_BR29, 538,   65, 54, 19,  4,  5,  6,  0},
{ Marklin8805_BR29, 670,   85, 71, 53, 33, 12,  6,  6},

{ Marklin8821_V200, 692,   88, 90, 59, 45, 23, 17, 17},
{ Marklin8821_V200, 761,   81, 67, 47, 32, 17, 17, 16},
{ Marklin8821_V200, 788,   79, 67, 53, 33, 18, 13, 13},

{ loco_unknown,       0,    0,  0,  0,  0,  0,  0,  0}
};


};
