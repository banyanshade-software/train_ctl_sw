/*
 * presence_detection.c
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */


#include <stddef.h>
#include <memory.h>
#include "misc.h"

#include "presence_detection.h"

#include "msg/trainmsg.h"
#include "../../stm32dev/ina3221/ina3221.h"

static int init_done = 0;

static void presdect_init(void)
{
	init_done = 1;
	ina3221_init();
}

void presdect_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	if (!init_done) {
		presdect_init();
	}
	static int16_t val1[INA3221_NUM_VALS]={0};
	static int16_t val2[INA3221_NUM_VALS]={0};
	static uint8_t fdone = 1;
	static uint8_t step = 0xFF;

	static uint8_t presence[INA3221_NUM_VALS] = {0};

	if (!fdone) {
		itm_debug1("prd/ko", fdone);
		return;
	}
	int16_t *values;
	int16_t *future;

	switch (step) {
	default:
	case 0xFF:
		values = NULL;
		future = val1;
		step = 0;
		break;
	case 0:
		values = val1;
		future = val2;
		step = 1;
	case 1:
		values = val2;
		future = val1;
		step = 0;
	}
    ina3221_start_read(future, &fdone);

    // process values
    if (!values) return;

    for (int i = 0; i<INA3221_NUM_VALS; i++) {
    	int p = (abs(values[i])>7000) ? 1 : 0;
    	if (p == presence[i]) continue;
    	presence[i] = p;
    	// notify change
    	msg_64_t m;
    	m.from = MA_CANTON(localBoardNum, 0);
    	m.to = MA_CONTROL();
    	m.cmd = CMD_PRESENCE_CHANGE;
    	m.sub = i;
    	m.v1u = p;
    	mqf_write_from_canton(&m);
    }
}
