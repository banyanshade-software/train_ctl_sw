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
 * | TEXT1     | text3s   |
 *
 */

// display layout is stored as bytecode

#define		CODE_STR			0x00		// 7lsb index to ui_string

#define		CODE_ZONE_STATUS	0x80
#define 	CODE_ZONE_MODE		0x81
#define		CODE_ZONE_TEXT1		0x82
#define		CODE_ZONE_TEXT2		0x83
#define		CODE_ZONE_TEXT2s	0x84
#define		CODE_ZONE_TEXT3s	0x85

// followed by 1 byte
#define		CODE_DIGIT			0xC0
#define		CODE_DIR			0xC1	// direction indicatior (< o >)

// followed by 2 bytes
#define		CODE_UVAL			0xB0
#define		CODE_SVAL			0xB1
#define		CODE_GRAPH_LEVEL	0xB2


#define		CODE_NOP			0xFF

#define MAX_OPCODE_PER_DISPLAY	24
typedef uint8_t disp_op_codes_t[MAX_OPCODE_PER_DISPLAY];
#define MAX_DISPLAY 4
static disp_op_codes_t disp[MAX_DISPLAY];

static const char *ui_strings[] = {
/*0*/		"(c) Braun",
/*1*/		"Train ",
/*2*/		"Hello",
/*3*/		"Ready",
/*4*/		"P=",
/*5*/		__DATE__,
/*6*/		"Z-v0.1.01",
/*7*/		"...",
/*8*/		"...",
/*9*/		"...",
/*10*/		"Fwd",
/*11*/		"Rev",
/*12*/		"Stop",
};

static void sample_display(int numdisp)
{
	static int cnt=0;
	uint8_t *d = &disp[numdisp][0];
	memset(d, CODE_NOP, MAX_OPCODE_PER_DISPLAY);
	int i = 0;
	// implicit CODE_ZONE_STATUS
	d[i++] = CODE_STR | 1;
	d[i++] = CODE_DIGIT;
	d[i++] = (cnt %10); cnt++;
	d[i++] = CODE_NOP;

	d[i++] = CODE_ZONE_MODE;
	d[i++] = CODE_STR | 0;
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;

	d[i++] = CODE_ZONE_TEXT1;
	d[i++] = CODE_STR | 3;
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;
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
	d[i++] = CODE_ZONE_TEXT3s;
	d[i++] = CODE_STR | 6;
	d[i++] = CODE_NOP;
#endif
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;
	d[i++] = CODE_NOP;
}



#define TEXT_Y 12
#define RIGHT_X 64


static void dolayout(int numdisp)
{
	uint8_t *d = &disp[numdisp][0];
	ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
	FontDef *curfont = &Font_7x10;

	for (int i=0; i<MAX_OPCODE_PER_DISPLAY; i++) {
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
		case CODE_ZONE_TEXT2s:
			ssd1306_SetCursor(RIGHT_X, TEXT_Y);
			curfont = &Font_7x10;
			break;
		case CODE_ZONE_TEXT3s:
			ssd1306_SetCursor(RIGHT_X, TEXT_Y+10);
			curfont = &Font_7x10;
			break;
		case CODE_DIGIT:
			i++;
			ssd1306_WriteChar('0'+d[i], *curfont, White);
			break;
		case CODE_SVAL:
			i+=2;
			break;
		case CODE_UVAL:
			i+=2;
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
			}
		}
	}
	ssd1306_UpdateScreen();
}

void _test_new_disp(void)
{
	sample_display(0);
	dolayout(0);
}

#endif // TFT_DISP
