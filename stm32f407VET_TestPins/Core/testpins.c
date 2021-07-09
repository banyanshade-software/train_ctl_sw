/*
 * testpins.c
 *
 *  Created on: Jul 8, 2021
 *      Author: danielbraun
 */


#include <stdint.h>
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "../../stm32dev/disp_tft/ssd1306.h"
#include "disp.h"

extern TIM_HandleTypeDef htim4;



#define ENC_MUL2  		0
#define ENC_DIV2	 	1
#define ENC_MAX (((16*5)<<ENC_DIV2)>>ENC_MUL2)
#define MIDDLE_ZERO 4


static uint8_t needsrefresh_mask;

#define SET_NEEDSREFRESH(_i) do { needsrefresh_mask = (needsrefresh_mask | (1<<(_i)));} while(0)
#define NEEDSREFRESH(_i) ((needsrefresh_mask & (1<<(_i))) ? 1 : 0)

/*
static void i2c_ready(int a)
{

}
static void I2C_Scan(void)
{
    HAL_StatusTypeDef res;
    for(uint16_t i = 0; i < 128; i++) {
        res = HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 10);
        if(res == HAL_OK) {
        	i2c_ready(i);
        } else {
        }
    }
}
*/
static uint16_t get_rotary(TIM_HandleTypeDef *ptdef)
{
	uint16_t p = __HAL_TIM_GET_COUNTER(ptdef);
	if (p>0x7FFF) {
		p = 0;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	} else if (p>=ENC_MAX) {
		p=ENC_MAX;
		__HAL_TIM_SET_COUNTER(ptdef, p);
	}
	return ((p<<ENC_MUL2)>>ENC_DIV2);//>>1;
}

static const uint8_t skip[16*5] = {
	//  0  1  2  3    4  5  6  7      8  9  10 11   12 13 14 15
		0, 0, 0, 0,   0, 0, 0, 0,     0, 0, 0, 1,   1, 1, 1, 0,	//PA skip PA13,14 (SW), PA11,PA12 (usb)
		0, 0, 1, 1,   1, 0, 1, 1,     0, 0, 0, 0,   0, 0, 0, 0,	//PB skip PB6, PB7 (I2C1) PB3 (trace), PB2 (boot1), PB4 (jt)
		0, 0, 0, 0,   0, 0, 0, 0,     0, 0, 0, 0,   0, 0, 1, 1, //PC skip PC14 PC15 (rcc)
		0, 0, 0, 0,   0, 0, 0, 0,     0, 0, 0, 0,   1, 1, 0, 0, //PD skip pd12,pd13 (rot enc)
		0, 0, 0, 0,   0, 0, 0, 0,     0, 0, 0, 0,   0, 0, 0, 0, //PE
};

static const char * name[16*5] = {
		// 0     1     2       3        4     5       6      7          8       9    10    11        12      13    14    15
		"V11", "V10", "V21", "V20",   "V31", "V30", "LED1", "LED2",    "i3CL", "R1", "R2", "usb",    "usb",  "-",  "-", "R5",
		"V40", "PW41", "b1", "sw0",   "-",   "T2B", "i2c", "i2c",      "Tout", "T2B","PW22","PW21",  "T1A", "T2A", "PW52","PW51",
		"V00", "V01", "V41", "V51",   "V50", "T6A", "PW32","PW31",     "PW42", "i3sd","T7B","T7A",   "T5B", "T5A", "rcc","rcc",
		"ct32","ct31","ct41","ct33",  "ct43","ct42","T3A","T3B",       "ct11","ct13","ct12","ct21",  "ro1", "ro2", "ct22", "ct23",
		"T8A", "T8B", "ct03", "ct02", "T4A", "ct01", "T4B","ETR1",		"ct51","PW02","CT52","PW01", "ct53","PW11","PW12","T6B"
};

void StartUiTask(void *argument)
{
	//I2C_Scan();
	ssd1306_Init();
	uint16_t pp = 0;
	SET_NEEDSREFRESH(0);

	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

	int port = 0;
	int pin = 0;
	GPIO_TypeDef *portptr = NULL;

	ihm_setstrbase(name);
	ihm_setvar(0, 0, 0+'A');
	ihm_setvar(0, 1, 0);
	ihm_setvar(0, 2, 0);

	for (;;) {
		if (NEEDSREFRESH(0)) {
			disp_layout(0);
		}
		osDelay(50);
		static int cnt = 0;
		cnt++;
		if (0==(cnt%5)) {
			// void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
			if (portptr) {
				HAL_GPIO_TogglePin(portptr, (1<<pin));
			}
		}

		uint16_t p = get_rotary(&htim4);
		if (pp == p) continue;

		if (portptr) {
			//HAL_GPIO_WritePin(portptr, pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(portptr, (1<<pin), GPIO_PIN_SET);
		}

		if (p >= 16*5) p = 16*5-1;

		for (;skip[p]; p++);


		pp = p;
		port  = p / 16;
		pin = p  % 16;

		if ((0) || ((port==0)&&(pin>=6)&&(pin<=7))) {
			switch (port) {
			case 0: portptr = GPIOA; break;
			case 1: portptr = GPIOB; break;
			case 2: portptr = GPIOC; break;
			case 3: portptr = GPIOD; break;
			case 4: portptr = GPIOE; break;
			default:
				for (;;) {
					osDelay(1000);
				}
				break;
			}
		} else {
			portptr = NULL;
		}

		ihm_setvar(0, 0, port+'A');
		ihm_setvar(0, 1, pin);
		ihm_setvar(0, 2, p);
		SET_NEEDSREFRESH(0);
	}
}
