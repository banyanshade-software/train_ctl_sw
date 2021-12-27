/*
 * taskdisp.c
 *
 *  Created on: Nov 25, 2020
 *      Author: danielbraun
 */

#include <memory.h>
#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../trainctl_iface.h"
#include "../IHM/disp.h"
#include "../IHM/ihm.h"


#ifndef BOARD_HAS_TFT
#error BOARD_HAS_TFT not defined, remove this file from build
#endif



#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
//#error bin non
#endif
#include "taskdisp.h"

#include "cmsis_os.h"
#include "main.h"

#include "../../stm32dev/disp_tft/ssd1306_tests.h"
#include "../../stm32dev/disp_tft/ssd1306.h"
//#include "../BLE/bletask.h"

extern int num_train_periodic_control;

#if 0
#define MAX_DISP 1 // for now
static uint8_t display_addr[MAX_DISP] = {0}; // unused for now
static uint8_t needsrefresh_mask;

#define SET_NEEDSREFRESH(_i) do { needsrefresh_mask = (needsrefresh_mask | (1<<(_i)));} while(0)
#define NEEDSREFRESH(_i) ((needsrefresh_mask & (1<<(_i))) ? 1 : 0)

#endif
/// ----------------------------------

static void i2c_ready(_UNUSED_ int a)
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


/*
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

static void ui_msg5(uint8_t disp, char *m6);
static void ui_canton_pwm(uint8_t, uint8_t, int8_t);

static int test_mode = 0;

*/

void StartUiTask(_UNUSED_ void *argument)
{
	// init
	extern TIM_HandleTypeDef htim4;

	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);


	for (int i=0; i<MAX_DISP; i++) {
		I2C_Scan();
		ssd1306_Init();
	}
	for (;;) {
		static TickType_t lasttick = 0;
		vTaskDelayUntil(&lasttick, 100);
		//lasttick = HAL_GetTick(); done by vTaskDelayUntil
		ihm_runtick();
	}
}

#if 0



// -----------------------------------------------------------------------

/* screen is devided in 4 zones
 * [status]    [mode ]
 * [TEXT1 ]    [TEXT2]
 */

#define TEXT_Y 14
#define RIGHT_X 64

static inline void _clear_status(uint8_t dispnum)
{
	ssd1306_FillZone(0, 0, RIGHT_X, TEXT_Y, Black);
}

static inline void _clear_mode(uint8_t dispnum)
{
	ssd1306_FillZone(RIGHT_X, 0, RIGHT_X, TEXT_Y, Black);
}


static inline void _clear_text(uint8_t dispnum)
{
	ssd1306_FillZone(0, TEXT_Y, 128, 32-TEXT_Y, Black);
}


static inline void _clear_text1(uint8_t dispnum)
{
	ssd1306_FillZone(0, TEXT_Y, RIGHT_X, 32-TEXT_Y, Black);
}


static inline void _clear_text2(uint8_t dispnum)
{
	ssd1306_FillZone(RIGHT_X, TEXT_Y, RIGHT_X, 32-TEXT_Y, Black);
}

// -----------------------------------------------------------------------


static void _write_mode(uint8_t dispnum, char *mode)
{
	_clear_mode(dispnum);
	ssd1306_SetCursor(RIGHT_X, 0);
	ssd1306_WriteString(mode, Font_7x10, White);
}

static void _write_status(uint8_t dispnum, char *str)
{
	_clear_status(dispnum);
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(str, Font_7x10, White);
}

static void ui_write_mode(uint8_t dispnum)
{
	ssd1306_SetCursor(64,0);
	switch (test_mode) {
	case 0:
		_write_mode(dispnum, "    Ready");
		break;
	case 1:
	default:
		_write_mode(dispnum, "Test Blk");
		break;
	}
	SET_NEEDSREFRESH(dispnum);
}

static void ui_write_status(uint8_t dispnum, char *st)
{
	_write_status(dispnum, st);
	SET_NEEDSREFRESH(dispnum);
}


static void ui_msg5(uint8_t dispnum, char *txt)
{
	_clear_text1(dispnum);
	ssd1306_SetCursor(0, TEXT_Y);
	ssd1306_WriteNString(txt, 5, Font_11x18, White);
	SET_NEEDSREFRESH(dispnum);
	/*
	//ssd1306_UpdateScreen();
	ui_write_status();
	ui_write_mode();
	ssd1306_SetCursor(64,11);
	ssd1306_WriteString("Hop Hop Hop", Font_7x10, White);
	ssd1306_SetCursor(64,22);
	ssd1306_WriteString(__DATE__, Font_7x10, White);

	if ((0)) {
		static int cnt = 0;
		cnt ++;
		if (cnt%7) ssd1306_FillZone(35, 13, 47, 11, White);
		else ssd1306_DrawRectangle(35, 13, 35+47, 13+11, White);
	}
	*/

}


static void ui_canton_pwm(uint8_t from, uint8_t v1u, int8_t v2)
{
	char msg[16];
	int i = 0;
	if (MA_BROADCAST == from) {
		msg[i++] = 'A';
	} else {
		msg[i++] = (from & 0x07) + '0';
	}
	msg[i++] = ' ';
	msg[i++] = 'V';
	msg[i++] = v1u + '0';
	msg[i++] = ' ';
	msg[i++] = 'P';
	msg[i++] = (v2 > 0) ? '+' : '-';
	write_num(msg+i, abs(v2), 3);
	i+=3;
	msg[i++]='\0';

	_clear_text(0);
	ssd1306_SetCursor(0,TEXT_Y);
	ssd1306_WriteString(msg, Font_11x18, White);
	SET_NEEDSREFRESH(0);
}

// ------------------------------------------------------------------



static void ui_process_msg(void)
{
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ui(&m);
		if (rc) break;
		if (IS_CONTROL_T(m.from) || IS_TRAIN_SC(m.from)) {
			static char t[3] = "Tx";
			t[1] = (m.from & 0x7) + '0';
			ui_write_status(0, t);
		} else if (IS_CANTON(m.from)) {
			static char t[] = "Blk--";
			t[4] = (m.from & 0x7) + '0';
			t[3] = (MA_2_BOARD(m.from))+'0';
			ui_write_status(0, t);
		} else if (IS_TURNOUT(m.from)) {
			static char t[] = "Trn--";
			t[4] = (m.from & 0x7) + '0';
			t[3] = (MA_2_BOARD(m.from))+'0';
			ui_write_status(0, t);
		} else {
			ui_write_status(0, "...");
		}
		switch(m.cmd) {
        case CMD_TEST_MODE:
            test_mode = m.v1u;
            ui_write_mode(0);
    		ui_msg5(0, "T");
            break;
        case CMD_SETVPWM:
        	if (test_mode) ui_canton_pwm(m.from, m.v1u, m.v2);
        	break;
        }
		if (IS_UI(m.to)) {
			int dn = m.to & 0x1F;
			if (dn != 1) {
				itm_debug1(DBG_UI, "?dn", dn);
				continue;
			}
			switch (m.cmd) {
			case CMD_UI_MSG5:
				ui_msg5(dn, (char *) m.rbytes+1);
				break;
			default:
				itm_debug1(DBG_UI, "cmd?", m.cmd);
				break;
			}
		} else {
			itm_debug1(DBG_UI, "non ui msg", 0);
		}
	}
}

static void ui_scan_inputs(void)
{
}

// ---------------------------------------------------------------------------------------------

void taskdisp(void)
{
	int numdisp=0;

	ssd1306_Fill(Black);
	ui_write_mode(0);
	ui_write_status(0, "DBN-Z");
	ssd1306_SetCursor(RIGHT_X, 32-10);
	ssd1306_WriteString(__DATE__, Font_7x10, White);
	ui_msg5(0, "INIT");

	for (;;) {
#if 0
		uint32_t notif;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		if (notif != NOTIF_TICKUI) {
			itm_debug1(DBG_ERR|DBG_UI, "notif?", notif);
		}
		if (!(notif & NOTIF_TICKUI)) continue;
#else
		static TickType_t lasttick = 0;
		vTaskDelayUntil(&lasttick, 100);
		//lasttick = HAL_GetTick();

#endif
		if ((1)) {
			_test_new_disp();
			continue;
		}
		ui_scan_inputs();
		ui_process_msg();
		for (int i=0; i<MAX_DISP; i++) {
			if (NEEDSREFRESH(i)) {
				ssd1306_UpdateScreen();
			}
		}
		needsrefresh_mask = 0;
		osDelay(200);
	}
/*
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
	*/
}

#if 0

/// --------
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

extern RTC_HandleTypeDef hrtc; // from main.c

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
#endif


#endif


