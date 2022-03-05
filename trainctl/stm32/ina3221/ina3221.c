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
#include "../../msg/trainmsg.h"
#include "../../oscillo/oscillo.h""


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

#ifndef INA3221_TASKRD
#error hu?
#endif

static void bkpoint(int loc, int err)
{
	itm_debug2(DBG_ERR|DBG_INA3221, "INA ERR", loc, err);
}


static int ina3221_write16(int a, int reg, uint16_t v);
static uint16_t ina3221_read16(int a, int reg);

static void ina3221_init_and_configure(void);

// ----------------------------------------------------------------------------------



static uint16_t values[INA3221_NUM_VALS*2] = {0};
volatile uint16_t *cur_values = values;

extern osThreadId_t ina3221_taskHandle;
static int lastErr = 0;
static int cvrf_dev = 0;

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

static int _trig(int dev);
static void _reg_read(int dev, int reg);
static void _read_complete(int err);
static int _next_dev(int d);


static void handle_ina_notif(uint32_t notif);
static void run_ina_task(void);

static volatile uint16_t mask_en_val = 0;
static 	ina_state_t state = state_idle;

volatile int16_t oscillo_ina0;
volatile int16_t oscillo_ina1;
volatile int16_t oscillo_ina2;


void ina3221_task_start(_UNUSED_ void *argument)
{
	if (DISABLE_INA3221) {
		for(;;) {
			osDelay(1);
		}
	}
	run_ina_task();
}


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
		itm_debug1(DBG_INA3221, "RDcpl", state);

		if ((state >= state_rd_0) && (state <= state_rd_11)) {
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




static int _next_dev(int dev)
{
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



static void _read_complete(_UNUSED_ int err)
{
	uint16_t *valu = (uint16_t *) cur_values;
	int16_t  *vals = (int16_t *) cur_values;
	static int8_t presence[INA3221_NUM_VALS] = {0};

	for (int i = 0; i<INA3221_NUM_VALS; i++) {
			valu[i] = __builtin_bswap16(valu[i]);
	}
	msg_64_t m;

	oscillo_ina0 = vals[0];
	oscillo_ina1 = vals[1];
	oscillo_ina2 = vals[2];

	switch (run_mode) {
	default:
		break;
	case runmode_detect1:
		itm_debug1(DBG_INA3221, "D/1", vals[1]);
		m.from =  MA_CANTON(localBoardNum, 0);
		m.to = MA_UI(1);
		m.cmd = CMD_INA3221_VAL1;
		m.subc = 1;
		m.v1 = vals[1];
		mqf_write_from_ina3221(&m);
		break;
	case runmode_normal:
		for (int i = 0; i<INA3221_NUM_VALS; i++) {
			if ((i<=2)) itm_debug2(DBG_INA3221, "ina val", i, vals[i]);
			int p = (abs(vals[i])>1000) ? 1 : 0;
			if (p == presence[i]) continue;
			presence[i] = p;
			itm_debug3(DBG_INA3221|DBG_PRES, "PRSCH", i,p, vals[i]);
			// notify change
			m.from = MA_CANTON(localBoardNum, 0);
			m.to = MA_CONTROL();
			m.cmd = CMD_PRESENCE_SUB_CHANGE;
			m.subc = i;
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

#if 0
/*
 * unused, but could be (was) used to access I2C from interrupt
 * normal HAL_I2C_Mem_Write_* and HAL_I2C_Mem_Read_* contains busy wait
 */

_UNUSED_ static int ina3221_write16it(int a, int reg, uint16_t v);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT_NW(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT_NW(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);



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

    status = HAL_I2C_Mem_Write_IT_NW(&INA3221_I2C_PORT, a<<1, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&w16, 2);

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
#endif

