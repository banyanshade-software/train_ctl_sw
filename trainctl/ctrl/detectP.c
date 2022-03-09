//
//  detectP.c
//  train_throttle
//
//  Created by Daniel BRAUN on 21/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//


#include "misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#include "ctrl.h"
#include "detectP.h"

#include "../utils/measval.h"


#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

/*
 train detection
 */


#define NUM_FREQS 7
static const freqs[NUM_FREQS] = { 100, /*400, 800,*/ 1000, 2000, 3000, 4000, 6000, 8000 };


static int detect_state = 0;
static int detect_canton = -1;
static uint32_t detect_ltick = 0;


typedef struct {
	measval_t ina;
	measval_t voff;
	measval_t von;
} detect_stat_t;

static detect_stat_t dst = {0};

#define MAX_CANTON_FOR_DETECTION 8

static detect_stat_t allstats[MAX_CANTON_FOR_DETECTION][NUM_FREQS] = {0};

static int save_freq = 0;
static int freqindex = 0;

void set_pwm_freq(int freqhz, int crit);
int get_pwm_freq(void);

#define MEAS_DURATION  500 //tick = ms
#define RELAX_DURATION 200




// ---------------------------------------------

static int setfreqi(int fi)
{
	if (fi >= NUM_FREQS) return -1;
	set_pwm_freq(freqs[fi], 1);
	return 0;
}

void detect2_init(void)
{
    detect_state = 0;
    detect_canton = -1;
    detect_ltick = 0;
    save_freq = get_pwm_freq();
    freqindex = 0;
    setfreqi(freqindex);
}

// ---------------------------------------------



/*
typedef struct {
	int16_t avg_von;
	int16_t avg_bemf;
	int16_t min_bemf;
	int16_t max_bemf;
} bemf_anal_t;

static bemf_anal_t bemf_anal[4][10] = {0};
*/


static  void analyse_bemf(int cnum, int frequi)
{
    if (!dst.voff.count) {
        itm_debug1(DBG_DETECT|DBG_ERR, "no bemf", detect_canton);
        return;
    }
    int freq = get_pwm_freq();
    int rf = dst.voff.count * 1000 / MEAS_DURATION;
    itm_debug1(DBG_DETECT, "Z canton", detect_canton);
    itm_debug3(DBG_DETECT, "Z freq", freq, rf, dst.voff.count);
    itm_debug3(DBG_DETECT, "Z bemf", dst.voff.min, measval_avg(&dst.voff), dst.voff.max);
    itm_debug3(DBG_DETECT, "Z von",  dst.von.min, measval_avg(&dst.von), dst.von.max);
    itm_debug3(DBG_DETECT, "Z ina",  dst.ina.min, measval_avg(&dst.ina), dst.ina.max);


    if (cnum>=4) return;
    if (frequi>=10) return;
    /*bemf_anal_t *p = &bemf_anal[cnum][frequi];
    p->avg_von = dst.detect_sum_von/dst.detect_count_bemf;
    p->avg_bemf =  dst.detect_sum_bemf/dst.detect_count_bemf;
    p->min_bemf = dst.detect_min_bemf;
    p->max_bemf = dst.detect_max_bemf;*/
}

void analyse_bemf_final(void)
{
	itm_debug1(DBG_CTRL, "job here", 0);
}
void detect2_process_tick(uint32_t tick)
{
	if (tick<6000) return;

	static int freqi = 0;
	if (0) {
	} else if (-1 == detect_state) {
		return;
	} else if (0 == detect_state) {
        detect_canton++;
        lsblk_num_t lsb = any_lsblk_with_canton(detect_canton);

        if (lsb.n<0) {
            detect_state = -1;
            // done for this freq
            freqindex++;
            int rc = setfreqi(freqindex);

            if (rc<0) {
            	// all done
                detect_state = -1;
				set_pwm_freq(save_freq, 1);
				itm_debug2(DBG_DETECT, "ALL DONE", 0, 0);

				analyse_bemf_final();

		        msg_64_t m = {0};

		        m.cmd = CMD_UI_DETECT;
		        m.v1 = -1;
		        m.v2 = 0;
		        m.to = MA_UI(UISUB_TFT);
		        mqf_write_from_ctrl(&m);

		        if ((1)) {
		        	//osDelay(500);
		        	msg_64_t m;
		        	m.from = MA_BROADCAST;
		        	m.to = MA_BROADCAST;
		        	m.cmd = CMD_SETRUN_MODE;
		        	m.v1u = runmode_normal;

		        	mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl
		        }

		        return;
            }

            itm_debug3(DBG_DETECT, "*freq", freqindex, freqs[freqindex], get_pwm_freq());
            detect_canton = 0;

        }

        // start detection on new canton
        detect_ltick = tick;
        itm_debug1(DBG_DETECT, "DETECT", detect_canton);

        if ((1)) {
        	// start monitoring current (INA3221) on concerned sub blocks
        	uint16_t inas = get_ina_bitfield_for_canton(detect_canton);
            msg_64_t m = {0};
            m.from = MA_CONTROL_T(0);
            m.to = MA_INA3221_B(localBoardNum); // XXX board to be added
            m.cmd = CMD_START_INA_MONITOR;
            m.v1u = inas;
            mqf_write_from_ctrl(&m);
        }

        // start monitoring BEMF
        msg_64_t m = {0};
        m.from = MA_CONTROL_T(0);
        m.to = detect_canton; //MA_CANTON(0, canton);
        m.cmd = CMD_START_DETECT_TRAIN;
        m.v1u = 80; //%pwm
        mqf_write_from_ctrl(&m);

        // notify UI
        m.cmd = CMD_UI_DETECT;
        m.v1 = detect_canton;
        m.v2u = get_pwm_freq();;
        m.to = MA_UI(UISUB_TFT);
        mqf_write_from_ctrl(&m);

        detect_state = 1;
        memset(&dst, 0, sizeof(dst));
    }  else if (detect_state==1) {
        if (tick >= detect_ltick+MEAS_DURATION) {
        	// stop monitoring BEMF
            msg_64_t m = {0};
            m.to = detect_canton; //MA_CANTON(0, canton);
            m.from = MA_CONTROL_T(0);
            m.cmd = CMD_STOP_DETECT_TRAIN;
            mqf_write_from_ctrl(&m);
            itm_debug2(DBG_DETECT, "END", detect_canton, detect_ltick);
            detect_ltick = tick;
            detect_state = 2;

            if ((1)) {
            	// stop monitoring current (INA3221)
            	msg_64_t m = {0};
            	m.from = MA_CONTROL_T(0);
            	m.to = MA_INA3221_B(localBoardNum); // XXX board to be added
            	m.cmd = CMD_START_INA_MONITOR;
            	m.v1u = 0;
            	mqf_write_from_ctrl(&m);
            }
            // analyse results
            analyse_bemf(detect_canton, freqi);
        }
    } else if (detect_state==2) {
    	// relax time
        if (tick > detect_ltick + RELAX_DURATION) {
            detect_ltick = tick;
            // start again
            detect_state = 0;
        }
    }
}

void detect2_process_msg(msg_64_t *m)
{
    /*if (!IS_CANTON(m->from) ) {
    	return;
    } */
    switch (m->cmd) {
        case CMD_BEMF_NOTIF:
        	itm_debug2(DBG_DETECT, "bemf", m->v1, m->v2); //XXX
            if (detect_state != 1) {
                itm_debug2(DBG_DETECT, "bad state", detect_state, m->from);
                break;
            }
            if (m->from != detect_canton) {
                itm_debug2(DBG_DETECT, "bad from", detect_canton, m->from);
                break;
            }
            measval_addvalue(&dst.voff, m->v1);
            measval_addvalue(&dst.von, m->v2);
            if ((1)) {
            	if (!freqindex && (detect_canton == 0) && (dst.voff.count > 20) && (m->v1 > 2000)) {
            		extern volatile int oscillo_trigger_start;
            		oscillo_trigger_start = 1;
            	}
            }

            break;
        case CMD_INA_REPORT:
        	if (detect_state != 1) {
        		itm_debug2(DBG_DETECT, "bad stat", detect_state, m->from);
        		break;
        	}
        	// XXX assume only one INA here
        	measval_addvalue(&dst.ina, m->v1);
        	if ((0)) itm_debug2(DBG_DETECT, "current", m->subc, m->v1);
        	break;
        default:
            break;
    }
}


