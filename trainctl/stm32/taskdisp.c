/*
 * taskdisp.c
 *
 *  Created on: Nov 25, 2020
 *      Author: danielbraun
 */

#include <memory.h>
#include "../misc.h"
#include "../trainctl_iface.h"


#ifndef TFT_DISP
#error TFT_DISP not defined
#endif


#if TFT_DISP

#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#error bin non
#endif
#include "taskdisp.h"

#include "cmsis_os.h"
#include "main.h"

#include "../../stm32dev/disp_tft/ssd1306_tests.h"
#include "../../stm32dev/disp_tft/ssd1306.h"
#include "../BLE/bletask.h"

extern int num_train_periodic_control;


void StartUiTask(void *argument)
{
	//MX_USB_DEVICE_Init();
	taskdisp();
}

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


static void bcd_2_char(char *buf, int v, int nch)
{
	while (nch>0) {
		nch--;
		buf[nch] = '0' + (v & 0x0F);
		v = v >> 4;
	}
}

static char hexchr(int v)
{
	v = v & 0xF;
	if (v>9) {
		return 'A'+(v-10);
	}
	return '0'+v;
}

static void write_num(char *buf, uint32_t v, int ndigit)
{
	for (;ndigit>0; ndigit--) {
		buf[ndigit-1] = '0'+ (v % 10);
		v = v/10;
	}
}


static void disp_hop(void);
static void disp_clock(void);
static void disp_ble1(void);
static void disp_ble2(void);
static void disp_ticks(void);
static void disp_c0(void);

void taskdisp(void)
{
	int numdisp=0;
	I2C_Scan();
	ssd1306_Init();
	for (;;) {
		if ((0)) {
			osDelay(10000);
			continue;
		}
		ssd1306_Fill(Black);
		ssd1306_SetCursor(0,0);
		if (numdisp>4) numdisp=0;
		switch (numdisp) {
		case 0:
			disp_hop();
			break;
		case 1: // clock
			disp_clock();
			break;
		case 2: // BLE
			disp_ble1();
			break;
		case 3: //BLE2
			disp_ble2();
			break;
		case 4: // ticks
			disp_ticks();
			break;
		case 5: // canton 0
			disp_c0();
			break;
		}
		ssd1306_UpdateScreen();
		osDelay(1000);
		numdisp++;
	}
}

static void disp_c0(void)
{
	static char str[] = "C0 V xxx p yyy";
	ssd1306_WriteString(str, Font_7x10, White);
}

static void disp_ticks(void)
{
					   //012345678990
	static char str[] = "T xxx N yyyy";
	write_num(str+2,train_tick_last_dt ,3);
	write_num(str+8, train_ntick, 4);

	ssd1306_WriteString(str, Font_7x10, White);
}

static void disp_ble1(void)
{
	//ssd1306_WriteString("BLE ", Font_11x18, White);
	//   0123456789
	static char str[] = "cmd x r yy";
	str[4] =  '0' + num_cmd;
	str[8] =  '0' + (num_rx/10)%10;
	str[9] =  '0' + num_rx%10;
	ssd1306_WriteString(str, Font_11x18, White);
}

static void disp_ble2(void)
{
	// 012345678
	static char spd[] = "s sxx sxx";
	spd[2] = (ble_spd1>=0) ? ' ' : '-';
	int as = abs(ble_spd1);
	spd[3] = hexchr((as >> 4) & 0xF);
	spd[4] = hexchr(as & 0xF);
	spd[6] = (ble_spd2>=0) ? ' ' : '-';
	as = abs(ble_spd2);
	spd[7] = hexchr((as >> 4) & 0xF);
	spd[8] = hexchr(as & 0xF);
	ssd1306_WriteString(spd, Font_11x18, White);
}

static void disp_hop(void)
{
	ssd1306_WriteString("INA3221/23", Font_7x10, White);
	ssd1306_SetCursor(0,16);
	ssd1306_WriteString("16", Font_11x18, White);
}

static void disp_clock(void)
{
	RTC_DateTypeDef sDate = {0};
	RTC_TimeTypeDef sTime = {0};
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
	char buf[32];
	bcd_2_char(buf, sTime.Hours, 2);
	buf[2]=':';
	bcd_2_char(buf+3, sTime.Minutes, 2);
	buf[5]=':';
	bcd_2_char(buf+6, sTime.Seconds, 2);
	buf[8]='\0';
	ssd1306_Fill(Black);
	ssd1306_SetCursor(2,0);
	ssd1306_WriteString(buf, Font_11x18, White);

	switch (sDate.WeekDay) {
	case RTC_WEEKDAY_MONDAY:    memcpy(buf, "Lun", 3); break;
	case RTC_WEEKDAY_TUESDAY:   memcpy(buf, "Mar", 3); break;
	case RTC_WEEKDAY_WEDNESDAY: memcpy(buf, "Mer", 3); break;
	case RTC_WEEKDAY_THURSDAY:  memcpy(buf, "Jeu", 3); break;
	case RTC_WEEKDAY_FRIDAY:    memcpy(buf, "Ven", 3); break;
	case RTC_WEEKDAY_SATURDAY:  memcpy(buf, "Sam", 3); break;
	case RTC_WEEKDAY_SUNDAY:    memcpy(buf, "Dim", 3); break;
	}
	buf[3]=' ';
	bcd_2_char(buf+4, sDate.Date, 2);
	buf[6]=' ';
	switch (sDate.Month) {
	case RTC_MONTH_JANUARY:    memcpy(buf+7, "Jan", 3); break;
	case RTC_MONTH_FEBRUARY:   memcpy(buf+7, "Fev", 3); break;
	case RTC_MONTH_MARCH:      memcpy(buf+7, "Mar", 3); break;
	case RTC_MONTH_APRIL:      memcpy(buf+7, "Avr", 3); break;
	case RTC_MONTH_MAY:        memcpy(buf+7, "Mai", 3); break;
	case RTC_MONTH_JUNE:       memcpy(buf+7, "Jun", 3); break;
	case RTC_MONTH_JULY:       memcpy(buf+7, "Jul", 3); break;
	case RTC_MONTH_AUGUST:     memcpy(buf+7, "Aou", 3); break;
	case RTC_MONTH_SEPTEMBER:  memcpy(buf+7, "Sep", 3); break;
	case RTC_MONTH_OCTOBER:    memcpy(buf+7, "Oct", 3); break;
	case RTC_MONTH_NOVEMBER:   memcpy(buf+7, "Nov", 3); break;
	case RTC_MONTH_DECEMBER:   memcpy(buf+7, "Dec", 3); break;
	}
	buf[10]=' ';
	buf[11]='2';
	buf[12]='0';
	bcd_2_char(buf+13, sDate.Year, 2);
	buf[15]='\0';
	ssd1306_SetCursor(20,16);
	ssd1306_WriteString(buf, Font_7x10, White);

	/*ssd1306_SetContrast(0x20);
		ssd1306_UpdateScreen();
		osDelay(1000*1);
		ssd1306_SetContrast(0xFF);
		osDelay(1000*1);
	 */
}

#if 0
		if ((1)) continue;

		ssd1306_Fill(Black);

		for (int i=0; i<32; i++) {
			ssd1306_Fill(Black);
			ssd1306_SetCursor(2,0);
			char s[4]="L00";
			s[1]='0'+i/10;
			s[2]='0'+i%10;
			ssd1306_WriteString(s, Font_11x18, White);
			ssd1306_Line(0,i, 127, i, White);
			ssd1306_UpdateScreen();
			osDelay(200);
		}
		for (int i=0; i<128; i++) {
			ssd1306_Fill(Black);
			ssd1306_SetCursor(2,0);
			char s[5]="C000";
			s[1]='0'+i/100;
			s[2]='0'+(i%100)/10;
			s[3]='0'+i%10;
			ssd1306_WriteString(s, Font_11x18, White);
			ssd1306_Line(i,0, i, 63, White);
			ssd1306_UpdateScreen();
			osDelay(200);
		}
	}
}

#endif





#endif 	// TFT_DISP
