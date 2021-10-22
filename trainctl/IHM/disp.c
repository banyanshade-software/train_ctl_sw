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


//static uint8_t display_addr[MAX_DISP] = {0}; // unused for now

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
#define 	CODE_ZONE_TEXT0Ls	CODE_ZONE_STATUS
#define 	CODE_ZONE_MODE		0x81
#define		CODE_ZONE_TEXT0Rs	CODE_ZONE_MODE
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

// followed by 1 byte, index to reg table, where a 8 or 16bit value is read

#define		CODE_DIGIT			0xC0
#define		CODE_DIR			0xC1	// direction indicatior (< o >)
#define		CODE_UVAL			0xC2
#define		CODE_UVAL4			0xC3
#define		CODE_UVAL1000       0xC4	// millivolt display (fixed point)
#define		CODE_SVAL			0xC5
#define		CODE_SVAL4			0xC6
#define		CODE_SVAL1000		0xC7	// millivolt, signed
#define		CODE_GRAPH_LEVEL	0xC8
#define		CODE_GRAPH_SLEVEL	0xC9
#define		CODE_SPTR			0xCA	// strnum in register


#define		CODE_END			0xFE
#define		CODE_NOP			0xFF

#define MAX_OPCODE_PER_DISPLAY	24

//typedef uint8_t disp_op_codes_t[MAX_OPCODE_PER_DISPLAY];
//static disp_op_codes_t disp[MAX_DISPLAY];



static const uint8_t *disp[MAX_DISP] = {NULL};


static const uint8_t layout_off[] = {
		CODE_ZONE_STATUS,	CODE_STR|2,
		CODE_ZONE_TEXT1,	CODE_STR|0,
		CODE_END
};

static const uint8_t default_layout[] = {
		CODE_ZONE_STATUS, 	CODE_STR|0,
		CODE_ZONE_TEXT1s,	CODE_STR|1,
		CODE_ZONE_TEXT2s,	CODE_STR|2,
		CODE_END
};

static const uint8_t layout_init[] = {
		CODE_ZONE_STATUS, 	CODE_STR|0,
		CODE_ZONE_TEXT1s,	CODE_STR|1,
		CODE_ZONE_TEXT2s,	CODE_STR|2,
		CODE_END
};

static const uint8_t layout_manual[] = {
		CODE_ZONE_STATUS, 	CODE_STR|4, CODE_DIGIT, 0,
		CODE_ZONE_MODE,     CODE_STR|5, CODE_DIR, 4, CODE_STR|20, CODE_SPTR,3,
		CODE_ZONE_TEXT1,  	CODE_STR|9, CODE_SVAL, 1, CODE_STR|6,
		CODE_ZONE_TEXT2s, 	CODE_GRAPH_SLEVEL, 2,
		//CODE_ZONE_TEXT3s,	CODE_STR|2,
		CODE_END
};

static const uint8_t layout_auto[] = {
		CODE_ZONE_STATUS, 	CODE_STR|4, CODE_DIGIT, 0,
		CODE_ZONE_MODE,     CODE_STR|7, CODE_DIR, 4, CODE_STR|20, CODE_SPTR,3,
		CODE_ZONE_TEXT1,  	CODE_STR|8,
		CODE_ZONE_TEXT2s, 	CODE_GRAPH_SLEVEL, 2,
		CODE_END
};

static const uint8_t layout_ina3221_i2c[] = {
		CODE_ZONE_TEXT0Ls, CODE_STR|21, CODE_DIGIT, 0,
		CODE_ZONE_TEXT0Rs, CODE_STR|22, CODE_DIGIT, 1,
		CODE_ZONE_TEXT1s,  CODE_STR|23, CODE_DIGIT, 2,
		CODE_ZONE_TEXT2s,  CODE_STR|24, CODE_DIGIT, 3,
		CODE_ZONE_TEXT3s,   CODE_STR|25,
		CODE_ZONE_TEXT3s,	CODE_STR|26,
		CODE_END
};


static const uint8_t layout_ina3221_val[] = {
		CODE_ZONE_TEXT0Ls, CODE_SVAL4,  0, /*CODE_STR|7,*/ CODE_SVAL4,  1, /*CODE_STR|7,*/ CODE_SVAL4,  2,
		CODE_ZONE_TEXT1s,  CODE_SVAL4,  3, /*CODE_STR|7,*/ CODE_SVAL4,  4, /*CODE_STR|7,*/ CODE_SVAL4,  5,
		CODE_ZONE_TEXT3s,  CODE_SVAL4,  6, /*CODE_STR|7,*/ CODE_SVAL4,  7, /*CODE_STR|7,*/ CODE_SVAL4,  8,
		CODE_END
};

static const uint8_t layout_detect1[] = {
		CODE_ZONE_STATUS, CODE_STR|27, CODE_SVAL1000, 5, // V=
		CODE_ZONE_MODE,   CODE_STR|28, CODE_SVAL1000, 6, // B=
		CODE_ZONE_TEXT2s, CODE_STR|29, CODE_SVAL4,   7, // I=
		CODE_ZONE_TEXT1,  CODE_STR|30, CODE_SVAL,    8, // PWM
		CODE_ZONE_TEXT4s, CODE_STR|31, CODE_UVAL,	 9, // VoltIdx
		CODE_END
};


void ihm_setlayout(int numdisp, int numlayout)
{
	const uint8_t *p = NULL;
	switch (numlayout) {
	case LAYOUT_DEFAULT: // default
		break;
	case LAYOUT_INIT:
		p = layout_init;
		break;
	case LAYOUT_MANUAL: // speed mode
		p = layout_manual;
		break;
	case LAYOUT_AUTO:
		p = layout_auto;
		break;
	case LAYOUT_INA3221_DETECT: // ina3221 I2C detection
		p = layout_ina3221_i2c;
		break;
	case LAYOUT_INA3221_VAL:
		p = layout_ina3221_val;
		break;
	case LAYOUT_OFF:
		p = layout_off;
		break;
	case LAYOUT_DETECT1:
		p = layout_detect1;
		break;
	default:
		itm_debug1(DBG_ERR|DBG_UI, "bad layout", numlayout);
		break;
	}
	disp[numdisp] = p;
}
/*
 *


 *
 */
static const char *ui_strings[] = {
/*0*/		"ATC-Z",
/*1*/		__DATE__,
/*2*/		"(c) Daniel Braun",
/*3*/		"Stop",

/*4*/		"train ",
/*5*/		"M ",
/*6*/		"%",
/*7*/		"A ",
/*8*/		"AUTO",
/*9*/		"V",
/*10*/		"off",			// train_state
/*11*/		"c1",
/*12*/		"c12",
/*13*/		"sta",
/*14*/		"blk",
/*15*/		"eot",
/*16*/		"...",
/*17*/		"...",
/*18*/		"...",
/*19*/		"...",
/*20*/		" ",	// space

/*21*/		"0x70:",			//ina3221 addr
/*22*/		"0x71:",
/*23*/		"0x72:",
/*24*/		"0x73:",
/*25*/		"ina3221",
/*26*/		"i2c",

/*27*/		"V=",
/*28*/		"B=",
/*29*/		"I=,",
/*30*/		"W ",
/*31*/		"Vidx=",

};


// ----------------------------------------------------------------

static uint16_t regs[DISP_MAX_REGS][MAX_DISP];

void ihm_setvar(int numdisp, int varnum, uint16_t val)
{
	if (varnum>DISP_MAX_REGS) return;
	if (numdisp>MAX_DISP) return;
	regs[varnum][numdisp] = val;
}
uint16_t ihm_getvar(int numdisp, int varnum)
{
	if (varnum>DISP_MAX_REGS) return 0;
	if (numdisp>MAX_DISP) return 0;
	return regs[varnum][numdisp];
}

#define _GET_REG(_disp, _reg) (ihm_getvar(_disp, _reg))

// ----------------------------------------------------------------



#define TEXT_Y 12
#define RIGHT_X 64

//static void write_num(char *buf, uint32_t v, int ndigit);
static void write_unum(uint16_t v, FontDef *curfont);
static void write_unum4(uint16_t v, FontDef *curfont);
static void write_unum1000(uint16_t v, FontDef *curfont);
static void write_snum(int16_t v, FontDef *curfont);
static void write_snum4(int16_t v, FontDef *curfont);
static void write_snum1000(int16_t v, FontDef *curfont);
static void write_bargraph(int16_t v, int16_t min, int16_t max);
static void write_sbargraph(int16_t v, int16_t min, int16_t max);

void disp_layout(int numdisp)
{
	uint32_t t0 = HAL_GetTick();
	const uint8_t *d = disp[numdisp];
	if (!d) d = default_layout;
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	FontDef *curfont = &Font_7x10;
	uint16_t v16u;
	int16_t v16s;
	//uint16_t *puval;
	static uint16_t last_dur1=0;
	static uint16_t last_dur2=0;

	for (int i=0; i<MAX_OPCODE_PER_DISPLAY; i++) {
		if (CODE_END == d[i]) break;
		if ((d[i] & 0x80)==0) { // CODE_STR
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
#ifdef SSD1306_INCLUDE_FONT_6x8
			ssd1306_SetCursor(0, TEXT_Y+12);
			curfont = &Font_6x8;
#else
			ssd1306_SetCursor(0, TEXT_Y+10);
			curfont = &Font_7x10;
#endif
			break;
		case CODE_ZONE_TEXT4s:
#ifdef SSD1306_INCLUDE_FONT_6x8
			ssd1306_SetCursor(RIGHT_X, TEXT_Y+12);
			curfont = &Font_6x8;
#else
			ssd1306_SetCursor(RIGHT_X, TEXT_Y+10);
			curfont = &Font_7x10;
#endif
			break;
#ifdef SSD1306_INCLUDE_FONT_16x26
		case CODE_ZONE_TEXTBIG:
			ssd1306_SetCursor(0,0);
			curfont = &Font_16x26;
			break;
#endif

		case CODE_SPTR:
			i++;
			v16u = (int16_t) _GET_REG(numdisp, d[i]);
			if (v16u<10) v16u = 10;
			if (v16u>100) v16u = 100;
			const char * stri = ui_strings[v16u];
			ssd1306_WriteString(stri, *curfont, White);
			break;
		case CODE_DIGIT:
			i++;
			v16u = (int16_t) _GET_REG(numdisp, d[i]);
			ssd1306_WriteChar('0'+(v16u & 0xF) , *curfont, White);
			break;
		case CODE_SVAL:
			i++;
			v16s = (int16_t) _GET_REG(numdisp, d[i]);
			write_snum(v16s, curfont);
			break;
		case CODE_SVAL4:
			i++;
			v16s = (int16_t) _GET_REG(numdisp, d[i]);
			write_snum4(v16s, curfont);
			break;
		case CODE_SVAL1000:
			i++;
			v16s = (int16_t) _GET_REG(numdisp, d[i]);
			write_snum1000(v16s, curfont);
			break;
		case CODE_UVAL:
			i++;
			v16u = _GET_REG(numdisp, d[i]);
			write_unum(v16u, curfont);
			break;
		case CODE_UVAL4:
			i++;
			v16u = _GET_REG(numdisp, d[i]);
			write_unum4(v16u, curfont);
			break;
		case CODE_UVAL1000:
			i++;
			v16u = _GET_REG(numdisp, d[i]);
			write_unum1000(v16u, curfont);
			break;
		case CODE_GRAPH_LEVEL:
			i++;
			v16u = _GET_REG(numdisp, d[i]);
			write_bargraph(v16u, 0, 100);
			break;
		case CODE_GRAPH_SLEVEL:
			i++;
			v16s = _GET_REG(numdisp, d[i]);
			write_sbargraph(v16s, -100, 100);
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
			v16s = _GET_REG(numdisp, d[i]);
			if (v16s > 0) ssd1306_WriteChar('>', *curfont, White);
			else if (v16s < 0) ssd1306_WriteChar('<', *curfont, White);
			else ssd1306_WriteChar('|', *curfont, White);
			break;


		default:
			switch (d[i] & 0xC0) {
			default:
			case 0x80: break;
			case 0xC0: i++; break;
			//case 0xB0: i+=2; break;
			//case 0xD0: i+=4; break;
			}
		}
	}
	uint32_t t1 = HAL_GetTick();
	ssd1306_UpdateScreen();
	uint32_t t2 = HAL_GetTick();
	last_dur1 = t2-t0;
	last_dur2 = t2-t1;
}

/*
void _test_new_disp(void)
{
	sample_display(0);
	dolayout(0);
}
*/

//

_UNUSED_ static void bcd_2_char(char *buf, int v, int nch)
{
	while (nch>0) {
		nch--;
		buf[nch] = '0' + (v & 0x0F);
		v = v >> 4;
	}
}

_UNUSED_ static char hexchr(int v)
{
	v = v & 0xF;
	if (v>9) {
		return 'A'+(v-10);
	}
	return '0'+v;
}

/*
static void write_num(char *buf, uint32_t v, int ndigit)
{
	for (;ndigit>0; ndigit--) {
		buf[ndigit-1] = '0'+ (v % 10);
		v = v/10;
	}
}
*/

static void _write_unum(uint16_t v, FontDef *curfont, uint8_t hzero, uint8_t fp1000)
{
	int f = 0;
	int ns = 1000;
	if (fp1000) {
		ns = 1000;
		hzero = 1;
	}
	for (int i=ns;i>0; i = i /10) {
		int n = v/i;
		if (!n && !f && (i>1)) {
			if (!hzero) continue;
			if (hzero == ' ') {
				ssd1306_WriteChar(' ', *curfont, White);
				continue;
			}
		}
		ssd1306_WriteChar(n+'0', *curfont, White);
		if ((f==0) && fp1000) {
			ssd1306_WriteChar(',', *curfont, White);
			hzero = 1;
		}
		f++;
		v = v - i*n;
	}
}
static void write_unum(uint16_t v, FontDef *curfont)
{
	_write_unum(v, curfont, 0, 0);
}
static void write_unum4(uint16_t v, FontDef *curfont)
{
	if (v>9999) v=9999;
	_write_unum(v, curfont, 1, 0);
}
static void write_unum1000(uint16_t v, FontDef *curfont)
{
	if (v>9999) v=9999;
	_write_unum(v, curfont, 1, 1);
}
static void write_snum(int16_t v, FontDef *curfont)
{
	if ((v<-5000)||(v>5000)) {
		itm_debug1(DBG_UI|DBG_ERR, "strange here", v);
	}
	if (v < 0) {
		ssd1306_WriteChar('-', *curfont, White);
	} else {
		ssd1306_WriteChar('+', *curfont, White);
	}
	write_unum(abs(v), curfont);
}

static void write_snum4(int16_t v, FontDef *curfont)
{
	if (v<-9999) v=-9999;
	if (v>9999) v=9999;
	if (v < 0) {
		ssd1306_WriteChar('-', *curfont, White);
	} else {
		ssd1306_WriteChar('+', *curfont, White);
	}
	_write_unum(abs(v), curfont, 1, 0);
}

static void write_snum1000(int16_t v, FontDef *curfont)
{
	if (v<-9999) v=-9999;
	if (v>9999) v=9999;
	if (v < 0) {
		ssd1306_WriteChar('-', *curfont, White);
	} else {
		ssd1306_WriteChar('+', *curfont, White);
	}
	_write_unum(abs(v), curfont, 1, 1);
}

static void write_bargraph(int16_t v, int16_t min, int16_t max)
{
	uint8_t x0 = ssd1306_GetCursorX();
	uint8_t y0 = ssd1306_GetCursorY();
	const uint8_t w = 50;
	const uint8_t h = 11; //y0+=3;
	ssd1306_DrawRectangle(x0, y0, x0+w, y0+h, White);

	if (v>max) v=max;
	if (v<min) v=min;
	int l = ((int)w*(v-min))/(max-min);
	if (l>0) ssd1306_FillZone(x0, y0, l, h, White);
	if ((min<0) && (max>0)) {
		l = ((int)w*(0-min))/(max-min);
		/// TODO ?
	}
}



static void write_sbargraph(int16_t v, int16_t min, int16_t max)
{
	uint8_t x0 = ssd1306_GetCursorX();
	uint8_t y0 = ssd1306_GetCursorY();
	const uint8_t w = 50;
	const uint8_t h = 11; //y0+=3;
	ssd1306_DrawRectangle(x0, y0, x0+w, y0+h, White);

	if (v>max) v=max;
	if (v<min) v=min;
	int m = ((int)w*(0-min))/(max-min);
	int l = ((int)w*(v-min))/(max-min);
	if (l>m) ssd1306_FillZone(x0+m, y0, l-m, h, White);
	else ssd1306_FillZone(x0+l, y0, m-l, h, White);
	ssd1306_Line(x0+m, y0-1, x0+m, y0+h+2, White);
	if ((min<0) && (max>0)) {
		l = ((int)w*(0-min))/(max-min);
		/// TODO
	}
}


#endif // TFT_DISP
