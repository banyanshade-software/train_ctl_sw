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
#include "railconfig.h"


#if INA3221_TASK
void presdect_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
}
#else
static int init_done = 0;

static void presdect_init(void);


void presdect_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	if (DISABLE_INA3221) return;
	extern int disable_ina3221;
	if (disable_ina3221) return;
	if (!init_done) {
		presdect_init();
	}
	//if ((1)) return; // XXXXXX

	if ((0)) {
		static int cnt=0;
		if (cnt%100) return; // XXX TODO startup state machine
	}
	static int16_t val1[INA3221_NUM_VALS]={0};
	static int16_t val2[INA3221_NUM_VALS]={0};
	static uint8_t fdone = 1;
	static uint8_t step = 0xFF;

	static uint8_t presence[INA3221_NUM_VALS] = {0};

	if (!fdone) {
		itm_debug1(DBG_PRES|DBG_INA3221, "prd/ko", fdone);
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
		break;
	case 1:
		values = val2;
		future = val1;
		step = 0;
		break;
	}
    ina3221_start_read(future, &fdone);

    // process values
    if (!values) return;

    if ((0)) {
    	itm_debug1(DBG_INA3221, "prs stp", step);
    	itm_debug3(DBG_PRES|DBG_INA3221, "prs0", values[0], values[1], values[2]);
    	itm_debug3(DBG_PRES|DBG_INA3221, "prs1", values[3], values[4], values[5]);
    	itm_debug3(DBG_PRES|DBG_INA3221, "prs2", values[6], values[7], values[8]);
    }
    if ((1)) {
    	itm_debug2(DBG_PRES, "prs02", values[2], future[2]);
    }
    if ((1)) {
    	for (int i = 0; i<INA3221_NUM_VALS; i++) {
    		values[i] = __builtin_bswap16(values[i]);
    		itm_debug2(DBG_INA3221, "ina val", i, values[i]);
    		int p = (abs(values[i])>4000) ? 1 : 0;
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
    static int cnt=0;
    cnt++;
    if ((1) && ((cnt%50)==0)) {
    	msg_64_t m;
    	static int16_t v[12];
    	memcpy(v, values, 12*2);
    	for (int i=0; i<12; i++) {
    		if (abs(v[i]) > 400) {
    			itm_debug2(DBG_INA3221, "ina big", i, v[i]);
    		}
    	}
    	if ((1)) v[7] = cnt;
    	m.from = MA_CANTON(localBoardNum, 0);
    	m.to = MA_UI(1);
    	m.cmd = CMD_INA3221_REPORT;
    	m.v32u = (uint32_t) v;
    	mqf_write_from_canton(&m);
    }
}

static void presdect_init(void)
{
	init_done = 1;

	ina3221_init(0);
}
#endif
