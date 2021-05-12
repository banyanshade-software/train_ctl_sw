/*
 * disp.c
 *
 *  Created on: May 10, 2021
 *      Author: danielbraun
 */

#include <stdint.h>
#include <memory.h>

#include "trainctl_config.h"
#include "disp.h"
#include "../msg/trainmsg.h"


#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#else
#include "stm32f1xx_hal.h"
#error not tested
#endif
#else
#error should not be used in simu
#include "train_simu.h"
#endif



#include "misc.h"
#include "main.h"



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

#include "../../stm32dev/disp_tft/ssd1306.h"
#include "ihm_messages.h"


static uint8_t display_addr[MAX_DISP] = {0}; // unused for now

/*
 * for multiple display, we cannot store buffer for each display
 * rather we store text/graphical item layout
 *
 * each SSD1306 display is divided into zones than can be grouped according to different layout
 *
 * layout 0
 * | BIG TEXT             |
 *
 *
 * layout 1
 *
 * | status    | mode     |
 * |           |          |
 * | TEXT1     | TEXT2    | (text1 & Text2 in medium font, can be joined if no text2)
 *
 * layout 3
 * | status    | mode     |
 * |           | text2s   |
 * | TEXT1     | text4s   |
 *
 * layout 4
 * | status    | mode     |
 * | text1s    | text2s   |
 * | text3s    | text4s   |
 *
 * layout 5
 *  | HUGE TEXT |
 */

// display layout is stored as bytecode

#define		CODE_STR			0x00		// 7lsb index to ui_string

#define		CODE_ZONE_STATUS	0x80
#define 	CODE_ZONE_MODE		0x81
#define		CODE_ZONE_TEXT1		0x82
#define		CODE_ZONE_TEXT2		0x83
#define		CODE_ZONE_TEXT1s	0x84
#define		CODE_ZONE_TEXT2s	0x85
#define		CODE_ZONE_TEXT3s	0x86
#define		CODE_ZONE_TEXT4s	0x87
#ifdef SSD1306_INCLUDE_FONT_16x26
#define		CODE_ZONE_TEXTBIG	0x87
#else
#define CODE_ZONE_TEXTBIG CODE_ZONE_TEXT1
#endif

#define 	CODE_TIM4_CNT		0x88
#define 	CODE_PROFILE		0x8F

// followed by 1 byte
#define		CODE_DIGIT			0xC0
#define		CODE_DIR			0xC1	// direction indicatior (< o >)
// followed by 2 bytes
#define		CODE_UVAL			0xB0
#define		CODE_UVAL100        0xB2	// centivolt display (fixed point)
#define		CODE_SVAL			0xB3
#define		CODE_GRAPH_LEVEL	0xB4

// followed by ptr (4 bytes)
#define 	CODE_PSVAL			0xD0
#define		CODE_PUVAL			0xD1

#define		CODE_END			0xFE
#define		CODE_NOP			0xFF

#define MAX_OPCODE_PER_DISPLAY	24
typedef uint8_t disp_op_codes_t[MAX_OPCODE_PER_DISPLAY];
#define MAX_DISPLAY 4
static disp_op_codes_t disp[MAX_DISPLAY];


/*
 *


 *
 */
static const char *ui_strings[] = {
/*0*/		"TrCtl",		// IHMMSG_TRAINCTL_INIT
/*1*/		"Fwd ",			// IHMMSG_TRAINCTL_FWD
/*2*/		"Rev",			// IHMMSG_TRAINCTL_REV
/*3*/		"Stop",			// IHMMSG_TRAINCTL_STOP

/*4*/		"P=",
/*5*/		__DATE__,
/*6*/		"Z-v0.1.01",
/*7*/		"...",
/*8*/		"...",
/*9*/		"...",
/*10*/		"Fwd",
/*11*/		"Rev",
/*12*/		"Stop",
/*13*/		"V",
/*14*/		"t=",
};

static void sample_display(int numdisp)
{
	static int cnt=0;
	uint8_t *d = &disp[numdisp][0];
	memset(d, CODE_NOP, MAX_OPCODE_PER_DISPLAY);
	int i = 0;
	if ((0)) {
		d[i++] = CODE_ZONE_TEXTBIG;
		d[i++] = CODE_STR | 2;
		d[i++] = CODE_ZONE_TEXT3s;
		d[i++] = CODE_STR | 6;
		return;
	}
	// implicit CODE_ZONE_STATUS
	d[i++] = CODE_STR | 1;
	d[i++] = CODE_DIGIT;
	d[i++] = (cnt %10); cnt++;
	d[i++] = CODE_NOP;

	d[i++] = CODE_ZONE_MODE;
	d[i++] = CODE_STR | 14; /// 0;
	d[i++] = CODE_PROFILE; //d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;

	d[i++] = CODE_ZONE_TEXT1;
	d[i++] = CODE_STR | 13;
	d[i++] = CODE_TIM4_CNT;
	//d[i++] = CODE_UVAL;
	//uint16_t v = 56223;
	//memcpy(d+i, &v, 2); i+=2;
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;

#if 0
	d[i++] = CODE_ZONE_TEXT2;
	d[i++] = CODE_STR | 4;
	d[i++] = CODE_SVAL;
	int16_t v = 23;
	memcpy(d+i, &v, 2); i+=2;
#else
	d[i++] = CODE_ZONE_TEXT2s;
	d[i++] = CODE_STR | 5;
	d[i++] = CODE_ZONE_TEXT4s;
	d[i++] = CODE_STR | 6;
	d[i++] = CODE_NOP;
#endif
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;
	d[i++] = CODE_END;
}



#define TEXT_Y 12
#define RIGHT_X 64

//static void write_num(char *buf, uint32_t v, int ndigit);
static void write_unum(uint16_t v, FontDef *curfont);

void disp_layout(int numdisp)
{
	uint32_t t0 = HAL_GetTick();
	uint8_t *d = &disp[numdisp][0];
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	FontDef *curfont = &Font_7x10;
	uint16_t v16u;
	int16_t v16s;
	uint16_t *puval;
	static uint16_t last_dur1=0;
	static uint16_t last_dur2=0;

	for (int i=0; i<MAX_OPCODE_PER_DISPLAY; i++) {
		if (CODE_END == d[i]) break;
		if ((d[i] & 0x80)==0) {
			const char * stri = ui_strings[d[i]&0x7F];
			ssd1306_WriteString(stri, *curfont, White);
			continue;
		}
		switch (d[i]) {
		case CODE_NOP: break;
		case CODE_ZONE_STATUS:
			ssd1306_SetCursor(0, 0);
			curfont = &Font_7x10;
			break;
		case CODE_ZONE_MODE:
			ssd1306_SetCursor(RIGHT_X, 0);
			curfont = &Font_7x10;
			break;
		case CODE_ZONE_TEXT1:
			ssd1306_SetCursor(0, TEXT_Y);
			curfont = &Font_11x18;
			break;
		case CODE_ZONE_TEXT2:
			ssd1306_SetCursor(RIGHT_X, TEXT_Y);
			curfont = &Font_11x18;
			break;
		case CODE_ZONE_TEXT1s:
			ssd1306_SetCursor(0, TEXT_Y);
			curfont = &Font_7x10;
			break;
		case CODE_ZONE_TEXT2s:
			ssd1306_SetCursor(RIGHT_X, TEXT_Y);
			curfont = &Font_7x10;
			break;
		case CODE_ZONE_TEXT3s:
			ssd1306_SetCursor(0, TEXT_Y+10);
			curfont = &Font_7x10;
			break;
		case CODE_ZONE_TEXT4s:
			ssd1306_SetCursor(RIGHT_X, TEXT_Y+10);
			curfont = &Font_7x10;
			break;
#ifdef SSD1306_INCLUDE_FONT_16x26
		case CODE_ZONE_TEXTBIG:
			ssd1306_SetCursor(0,0);
			curfont = &Font_16x26;
			break;
#endif
		case CODE_DIGIT:
			i++;
			ssd1306_WriteChar('0'+d[i], *curfont, White);
			break;
		case CODE_SVAL:
			i+=2;
			break;
		case CODE_UVAL:
			memcpy(&v16u, d+i+1, 2);
			i+=2;
			write_unum(v16u, curfont);
			break;
		case CODE_PUVAL:
			memcpy(&puval, d+i+1, 4);
			i+=2;
			write_unum(*puval, curfont);
			break;
		case CODE_TIM4_CNT: {
			extern TIM_HandleTypeDef htim4;
			v16u = htim4.Instance->CNT;
			write_unum(v16u, curfont);
			break;
		}
		case CODE_PROFILE:
			write_unum((int16_t)last_dur1, curfont);
			ssd1306_WriteChar('/', *curfont, White);
			write_unum((int16_t)last_dur2, curfont);
			break;
		case CODE_DIR:
			i+=1;
			break;
		case CODE_GRAPH_LEVEL:
			i+=2;
			break;
			break;
		default:
			switch (d[i] & 0xF0) {
			default:
			case 0x80: break;
			case 0xC0: i++; break;
			case 0xB0: i+=2; break;
			case 0xD0: i+=4; break;
			}
		}
	}
	uint32_t t1 = HAL_GetTick();
	ssd1306_UpdateScreen();
	uint32_t t2 = HAL_GetTick();
	last_dur1 = t2-t0;
	last_dur2 = t2-t1;
}

void _test_new_disp(void)
{
	sample_display(0);
	dolayout(0);
}


//

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

static void write_unum(uint16_t v, FontDef *curfont)
{
	int f = 0;
	for (int i=10000;i>0; i = i /10) {
		int n = v/i;
		if (!n && !f && (i>1)) continue;
		f = 1;
		ssd1306_WriteChar(n+'0', *curfont, White);
		v = v - i*n;
	}
}

#endif // TFT_DISP
