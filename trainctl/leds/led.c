//
//  led.c
//  train_throttle
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//



//#include "../railconfig.h"
#include "../misc.h"
#include "../config/conf_led.h"
#include "led.h"
#include "ledio.h"


#ifndef BOARD_HAS_LED
#error BOARD_HAS_LED not defined, remove this file from build
#endif

/*
 LED bytecode is a sequence of 32 bits words
 8 bits of cmde | 24 bit sequence
 
 0 r r r r r r r | seq : sequence executed r+1 time; r=0x7F : infinty
 1 0 0 0 0 0 0 0 | w    : jump to word w
 1 0 0 0 0 0 0 1 | p    : jump to program P
 1 0 0 0 0 0 1 0 | n    : loop n time
 1 1 1 1 1 1 0 0 | seq  : end-loop
 1 1 1 1 1 1 1 s | -    : end prog, with val = s
 */

#define SEQ(_seq)       (((0x00)<<24) | (_seq))
#define R(_r, _seq)     (((_r)<<24) | (_seq))
#define INF(_seq)       (((0x7F)<<24) | (_seq))

#define JMP(_pc)        (((0x80)<<24) | (_pc))
#define PROG(_pn)       (((0x81)<<24) | (_pn))

#define RETURN(_val)    (((0xFE | ((_val) ? 1 : 0))<<24))
#define END_LOOP(_seq)  (((0xFC)<<24) | (_seq))
#define LOOP(_n)        (((0x81)<<24) | (_n))


static const uint32_t prog_on[] = {
    RETURN(1)
};
static const uint32_t prog_off[] = {
    RETURN(0)
};
static const uint32_t prog_25p[] = {
    INF( 0b000100010001000100010001)
};
static const uint32_t prog_50p[] = {
    INF( 0b010101010101010101010101)
};
static const uint32_t prog_test[] = {
    R(2, 0b000100000000000110000011),
    R(3, 0b000000000011111111111111),
    R(1, 0b110000000000001000000000),
    R(2, 0b101010101010101010101010),
    R(6, 0b111111111111111111111111),
    R(1, 0b101010101010101011111110),
	//R(3, 0b111111111111111111111111),
    //R(0, 0b101010101010101010101010),
    RETURN(1)
};
static const uint32_t prog_neon[] = {
    R(2, 0b000100000000000110000011),
    R(3, 0b000000000011111111111111),
    R(1, 0b110000000000001000000000),
    R(2, 0b101010101010101010101010),
    R(0, 0b111111111111111111110000),
    R(0, 0b001111111111000011001111),
    R(0, 0b101010111111011111111110),
	//R(3, 0b111111111111111111111111),
    //R(0, 0b101010101010101010101010),
    RETURN(1)
};

static const uint32_t prog_dimoff[] = {
    R(1, 0b111011101110111011101110), // 75%
    R(1, 0b110110110110110110110110), // 66%
    R(1, 0b110011001100110011001100), // 50%
    R(0, 0b000100010001000100010001), // 25%
    //R(1, 0b000000010000000100000001), // 12%
    RETURN(0)
};

static const uint32_t prog_dimon[] = {
	    R(0, 0b000100010001000100010001), // 25%
	    R(1, 0b110011001100110011001100), // 50%
	    R(1, 0b110110110110110110110110), // 66%
	    R(1, 0b111011101110111011101110), // 75%
		RETURN(1)
};

static const uint32_t prog_flash[] = {
		R(0, 0b000100010001000100010001), // 25%
		R(0, 0b110011001100110011001100), // 50%
		R(0, 0b111011101110111011101110), // 75%
	    R(0, 0b111111111111111111111111),
		R(0, 0b111011101110111011101110), // 75%
		R(0, 0b110011001100110011001100), // 50%
		R(0, 0b000100010001000100010001), // 25%
	    R(1, 0b000000000000000000000000),
		JMP(0)

};

static const uint32_t *progs[] = {
	/* 0 */ prog_off,
	/* 1 */ prog_25p,
	/* 2 */ prog_50p,
	/* 3 */ prog_on,
    /* 4 */ prog_off, // twice for easier test of prog_test
    /* 5 */ prog_test,
    /* 6 */ prog_neon, // same for now
    /* 7 */ prog_dimoff,
	/* 8 */ prog_dimon,
	/* 9 */ prog_flash
};
// ---------------------------------------

typedef struct {
    uint8_t prognum;
    uint8_t bit;
    uint16_t pc;
    uint16_t repeat_counter;
    // unimplemented :
    uint16_t loop_counter; //single loop for now
    uint16_t loop_pc;
} ledmachine_t;

static void reset_machine(ledmachine_t *state)
{
    state->prognum = 0xFF;
    state->bit = 0;
    state->pc = 0;
    state->repeat_counter = 0;
    state->loop_pc = 0;
    state->loop_counter = 0;
}
static void start_prog(ledmachine_t *state, uint8_t prognum)
{
    reset_machine(state);
    state->prognum = prognum;
}

static void err(void)
{
    
}

static uint8_t _run_one(ledmachine_t *state, uint8_t *rerun)
{
    *rerun = 0;
    if (state->bit) {
        state->bit--;
        uint8_t b = state->bit;
        uint32_t w = progs[state->prognum][state->pc];
        if (!state->bit) {
            if (!state->repeat_counter) {
                state->pc++;
            } else {
                if (state->repeat_counter != 0x7F) state->repeat_counter--;
                state->bit = 24;
            }
        }
        return (w & (1<<b)) ? 1 : 0;
    }
    uint32_t w = progs[state->prognum][state->pc];
    uint32_t opcode = w & 0xFF000000;
    if (0) {
    } else if (!(opcode & 0x80000000)) { //R()
        state->bit = 24;
        state->repeat_counter = (opcode >> 24) & 0x7F;
        *rerun = 1;
        return 0;
    } else if (0x80000000 == opcode) { // JMP()
        state->pc = w & 0x0000FFFF; // 0x00FFFFFF but PC is 16bits
        *rerun = 1;
        return 0;
    } else if (0x81000000 == opcode) { // PROG()
        start_prog(state, w & 0x000000FF);
        *rerun = 1;
        return 0;
    } else if (0xFE000000 == (w & 0xFE000000)) { // RETURN()
        uint8_t s = (w & 0x01000000) ?  1 : 0;
        state->prognum = 0xFF;
        state->pc = 0;
        return s;
    } else if (0x82000000 == opcode) { // LOOP()
        // unimplemented
        err();
    } else if (0xFC000000 == opcode) { // ENDLOOP()
        // unimplemented
        err();
    }
    err();
    // invalid opcode
    state->pc++;
    return 0;
}

static uint8_t run_ledmachine(ledmachine_t *state)
{
    if (state->prognum == 0xFF) return 0;
    uint8_t rerun;
    uint8_t rc;
    for (int c=0; ; c++) {
    	rc = _run_one(state, &rerun);
    	if (!rerun) break;
    	if (c>1) {
    		FatalError("LEDc", "c>1", Error_Ledc);
    	}
    }
    return rc;
}

static ledmachine_t leds[NUM_LEDS];

static int first = 1;

void led_reset_all(void)
{
    first = 1;
}
void led_start_prog(uint8_t lednum, uint8_t prognum)
{
    start_prog(&leds[lednum], prognum);
}

void led_run_all(void)
{
    if (first) {
        first = 0;
        for (int i=0; i<NUM_LEDS; i++) {
            reset_machine(&leds[i]);
        }
    }
    for (int i=0; i<NUM_LEDS; i++) {
        if (leds[i].prognum == 0xFF) continue;
        uint8_t v = run_ledmachine(&leds[i]);
        led_io(i, v);
        //printf("M%d: %d\n", i, v);
    }
}


