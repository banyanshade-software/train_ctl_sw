/*
 * misc.c
 *
 *  Created on: Sep 14, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "misc.h"
#ifndef TRAIN_SIMU
#include "main.h"
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif
#include <string.h>
#include <memory.h>


void flash_led(void)
{
#ifndef TRAIN_SIMU
	HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
	//HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
#endif
}




int trainctl_error(char l, int code, const char *msg)
{
	int16_t c = code;
	// add debug msg or notification here
	if (c >=0) c=-1;
	trainctl_notif('G', 0, 'E', (void *)&c, sizeof(c));
	return code;
}


// ITM helpers
// -------------------------------------------------------------------------

static void write_num(uint8_t *buf, uint32_t v, int ndigit)
{
	for (;ndigit>0; ndigit--) {
		buf[ndigit-1] = '0'+ (v % 10);
		v = v/10;
	}
}


int _write(int32_t file, uint8_t *ptr, int32_t len)
{
	for (int i = 0; i < len; i++)
	{
		ITM_SendChar(*ptr++);
	}
	return len;
}

static void _itm_debug3(const char *msg, int v1, int v2, int v3, int n);

void itm_debug1(const char *msg, int v)
{
	_itm_debug3(msg, v, 0, 0, 1);
}

void itm_debug2(const char *msg, int v1, int v2)
{
	_itm_debug3(msg, v1, v2, 0, 2);
}
void itm_debug3(const char *msg, int v1, int v2, int v3)
{
	_itm_debug3(msg, v1, v2, v3, 3);
}

static void _itm_debug3(const char *msg, int v1, int v2, int v3, int n)
{
	uint8_t buf[64];
	memset(buf, 0, sizeof(buf));
	write_num(buf, HAL_GetTick(), 7);
	buf[7]=':';
	strncpy(buf+8, msg, 12);
	uint8_t *p = buf+strlen(buf);
	if (!n--) goto done;
	*p = '/';
	itoa(v1, p+1, 10);
	p = buf+strlen(buf);
	if (!n--) goto done;
	*p = '/';
	itoa(v2, p+1, 10);
	p = buf+strlen(buf);
	if (!n--) goto done;
	*p = '/';
	itoa(v3, p+1, 10);
done:
	p = buf+strlen(buf);
	*p = '\n';
	_write(0, buf, strlen(buf));
}
// -------------------------------------------------------------------------

void __attribute__((weak)) trainctl_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen) 
{

}
void __attribute__((weak)) trainctl_notif2(uint8_t sel, uint8_t num, uint8_t cmd, char *msg, int32_t v1, int32_t v2, int32_t v3)
{

}
