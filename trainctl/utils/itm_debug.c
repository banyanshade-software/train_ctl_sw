/*
 * itm_debug.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


// ITM helpers
// -------------------------------------------------------------------------

#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include "misc.h"
#ifndef TRAIN_SIMU
#include "cmsis_os2.h"
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif
#include "itm_debug.h"

#ifndef TRAIN_SIMU
#include "main.h"
#ifdef STM32_F4
//#include "stm32f4xx_hal.h"
#else
//#include "stm32f1xx_hal.h"
#endif
//#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif


#ifdef TRAIN_SIMU
char* itoa (unsigned long long  value,  char str[],  int radix)
{
    char        buf [66];
    char*       dest = buf + sizeof(buf);
    int8_t     sign = 0;

    if (value == 0) {
        memcpy (str, "0", 2);
        return str;
    }

    if (radix < 0) {
        radix = -radix;
        if ( (long long) value < 0) {
            value = -value;
            sign = 1;
        }
    }

    *--dest = '\0';

    switch (radix)
    {
    case 16:
        while (value) {
            * --dest = '0' + (value & 0xF);
            if (*dest > '9') *dest += 'A' - '9' - 1;
            value >>= 4;
        }
        break;
    case 10:
        while (value) {
            *--dest = '0' + (value % 10);
            value /= 10;
        }
        break;

    case 8:
        while (value) {
            *--dest = '0' + (value & 7);
            value >>= 3;
        }
        break;

    case 2:
        while (value) {
            *--dest = '0' + (value & 1);
            value >>= 1;
        }
        break;

    default:            // The slow version, but universal
        while (value) {
            *--dest = '0' + (value % radix);
            if (*dest > '9') *dest += 'A' - '9' - 1;
            value /= radix;
        }
        break;
    }

    if (sign) *--dest = '-';

    memcpy (str, dest, buf +sizeof(buf) - dest);
    return str;
}
#endif





static void write_num(uint8_t *buf, uint32_t v, int ndigit)
{
	for (;ndigit>0; ndigit--) {
		buf[ndigit-1] = '0'+ (v % 10);
		v = v/10;
	}
}

#ifndef TRAIN_SIMU
int _write(int32_t file, uint8_t *ptr, int32_t len)
{
	for (int i = 0; i < len; i++)
	{
		ITM_SendChar(*ptr++);
	}
	return len;
}
#endif

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
	strncpy((char *)buf+8, msg, 12);
	uint8_t *p = buf+strlen((char *)buf);
	if (!n--) goto done;
	*p = '/';
	itoa(v1, (char *)p+1, 10);
	p = buf+strlen((char *)buf);
	if (!n--) goto done;
	*p = '/';
	itoa(v2, (char *)p+1, 10);
	p = buf+strlen((char *)buf);
	if (!n--) goto done;
	*p = '/';
	itoa(v3, (char *)p+1, 10);
done:
	p = buf+strlen((char *)buf);
	*p = '\n';
#ifndef TRAIN_SIMU
	_write(0, buf, strlen((char *)buf));
#else
    puts((char*)buf);
    //write(0, buf, strlen((char *)buf));
#endif
}
