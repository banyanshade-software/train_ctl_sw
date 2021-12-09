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



#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

/*
 train detection
 */

static int detect_state = 0;
static int detect_canton = -1;
static uint32_t detect_ltick = 0;
static int32_t detect_count_bemf = 0;
static int32_t detect_sum_bemf = 0;
static int32_t detect_min_bemf = 0;
static int32_t detect_max_bemf = 0;
static int32_t detect_sum_von = 0;
static int32_t detect_min_von = 0;
static int32_t detect_max_von = 0;

static int save_freq = 0;


void set_pwm_freq(int freqhz);
int get_pwm_freq(void);


void detect2_init(void)
{
    detect_state = 0;
    detect_canton = -1;
    detect_ltick = 0;

    save_freq = get_pwm_freq();
}

#define MEAS_DURATION 500 //tick = ms

typedef struct {
	int16_t avg_von;
	int16_t avg_bemf;
	int16_t min_bemf;
	int16_t max_bemf;
} bemf_anal_t;

static bemf_anal_t bemf_anal[4][10] = {0};

static  void analyse_bemf(int cnum, int frequi)
{
    if (!detect_count_bemf) {
        itm_debug1(DBG_DETECT|DBG_ERR, "no bemf", detect_canton);
        return;
    }
    int freq = get_pwm_freq();
    int rf = detect_count_bemf * 1000 / MEAS_DURATION;
    itm_debug3(DBG_DETECT, "Z freq", freq, rf, detect_count_bemf);
    itm_debug3(DBG_DETECT, "Z bemf --", detect_canton, freq, detect_sum_bemf/detect_count_bemf);
    itm_debug2(DBG_DETECT, "Z b min max", detect_min_bemf, detect_max_bemf);
    itm_debug3(DBG_DETECT, "Z von  --", detect_canton, freq, detect_sum_von/detect_count_bemf);
    itm_debug2(DBG_DETECT, "Z v min max", detect_min_von, detect_max_von);
    if (cnum>=4) return;
    if (frequi>=10) return;
    bemf_anal_t *p = &bemf_anal[cnum][frequi];
    p->avg_von = detect_sum_von/detect_count_bemf;
    p->avg_bemf =  detect_sum_bemf/detect_count_bemf;
    p->min_bemf = detect_min_bemf;
    p->max_bemf = detect_max_bemf;
}

void analyse_bemf_final(void)
{
	itm_debug1(DBG_CTRL, "job here", 0);
}
void detect2_process_tick(uint32_t tick)
{
	static int freqi = 0;
	if (0) {
	} else if (-1 == detect_state) {
		return;
	} else if (0 == detect_state) {
        detect_canton++;
        lsblk_num_t lsb = any_lsblk_with_canton(detect_canton);
        if (lsb.n<0) {
            detect_state = -1;
            // done
            int freq = get_pwm_freq();
            itm_debug1(DBG_DETECT, "done freq", freq);
            if (0) {
            } else if (freq<= 100) {
            	freqi=1;
            	freq = 200;
            } else if (freq<= 200) {
            	freqi=2;
            	freq = 500;
            } else if (freq<= 504) {
            	freqi=3;
            	freq = 1000;
            } else if (freq<= 1224) {
            	freqi=4;
            	freq = 2000;
            } else if (freq<= 2500) {
            	freqi=5;
            	freq = 3000;
            } else if (freq<= 2700) {
            	freqi=6;
            	freq = 4000;
            } else if (freq<= 4000) {
            	freqi=7;
            	freq = 8000;
            } else if (freq<= 9000) {
            	freqi=8;
            	freq = 10000;
            } else if (freq<= 16000) {
            	freqi=9;
                 freq = 20000;
            } else {
            	// all done
                detect_state = -1;
				set_pwm_freq(save_freq);
				itm_debug2(DBG_DETECT, "ALL DONE", 0, 0);

				analyse_bemf_final();

		        msg_64_t m = {0};

		        m.cmd = CMD_UI_DETECT;
		        m.v1 = -1;
		        m.v2 = 0;
		        m.to = MA_UI(UISUB_TFT);
		        mqf_write_from_ctrl(&m);

		        if ((1)) {
		        	msg_64_t m;
		        	m.from = MA_BROADCAST;
		        	m.to = MA_BROADCAST;
		        	m.cmd = CMD_SETRUN_MODE;
		        	m.v1u = runmode_normal;

		        	mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl
		        }

		        return;
            }
            set_pwm_freq(freq);
            itm_debug2(DBG_DETECT, "freq", freq, get_pwm_freq());
            detect_canton = 0;

        }
        detect_ltick = tick;
        msg_64_t m = {0};
        m.from = MA_CONTROL_T(0);
        m.to = detect_canton; //MA_CANTON(0, canton);
        itm_debug1(DBG_DETECT, "DETECT", detect_canton);
        m.cmd = CMD_START_DETECT_TRAIN;
        mqf_write_from_ctrl(&m);

        m.cmd = CMD_UI_DETECT;
        m.v1 = detect_canton;
        m.v2u = get_pwm_freq();;
        m.to = MA_UI(UISUB_TFT);
        mqf_write_from_ctrl(&m);

        detect_state = 1;
        detect_count_bemf = 0;
        detect_sum_bemf = detect_min_bemf = detect_max_bemf = 0;
        detect_sum_von  = detect_min_von  = detect_max_von  = 0;
        
    }  else if (detect_state==1) {
        if (tick >= detect_ltick+MEAS_DURATION) {
            msg_64_t m = {0};
            m.to = detect_canton; //MA_CANTON(0, canton);
            m.from = MA_CONTROL_T(0);
            m.cmd = CMD_STOP_DETECT_TRAIN;
            mqf_write_from_ctrl(&m);
            itm_debug1(DBG_DETECT, "END", detect_canton);
            detect_ltick = tick;
            detect_state = 2;

            analyse_bemf(detect_canton, freqi);
        }
    } else if (detect_state==2) {
        if (tick > detect_ltick + 100) {
            detect_state = 0;
        }
    }
}

void detect2_process_msg(msg_64_t *m)
{
    if (!IS_CANTON(m->from)) return;
    switch (m->cmd) {
        case CMD_BEMF_NOTIF:
            if (detect_state != 1) {
                itm_debug2(DBG_DETECT, "bad state", detect_state, m->from);
                break;
            }
            if (m->from != detect_canton) {
                itm_debug2(DBG_DETECT, "bad from", detect_canton, m->from);
                break;
            }
            // handle bemf notif
            int32_t b = m->v1;
            int32_t v = m->v2;
            if (!detect_count_bemf) {
                detect_max_bemf = detect_min_bemf = detect_sum_bemf = b;
                detect_max_von  = detect_min_von  = detect_sum_von  = v;
                detect_count_bemf = 1;
            } else {
                detect_count_bemf ++;
                detect_sum_bemf += b;
                detect_max_bemf = MAX(detect_max_bemf, b);
                detect_min_bemf = MIN(detect_min_bemf, b);
                detect_sum_von  += v;
                detect_max_von  = MAX(detect_max_von, v);
                detect_min_von  = MIN(detect_min_von, v);
            }
            break;

        default:
            break;
    }
}


