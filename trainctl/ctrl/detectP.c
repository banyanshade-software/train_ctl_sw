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


#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

#include "detectP.h"


#define MEAS_DURATION  300 //tick = ms
#define RELAX_DURATION 200

#if 0 // unmaintained

#include "ctrlP.h"
#include "../utils/measval.h"
#include "detect_loco.h"



/*
 train detection
 */


//static const int freqs[DETECT_NUM_FREQS] = { 100, 1000, 2000, 3000, 4000, 5000, 6000, 7000 };
static const int freqs[DETECT_NUM_FREQS] = { 100, 200, 400 };

#define USE_INA_FOR_DETECT 1

static int detect_state = 0;
static xblkaddr_t detect_canton = {.v=0xFF};
static uint32_t detect_ltick = 0;


typedef struct {
	measval_t ina;
	measval_t voff;
	measval_t von;
} detect_stat_t;

static detect_stat_t dst = {0};

#define MAX_CANTON_FOR_DETECTION 4

//static detect_stat_t allstats[MAX_CANTON_FOR_DETECTION][DETECT_NUM_FREQS] = {0};

static int save_freq = 0;
static int freqindex = 0;

void set_pwm_freq(int freqhz, int crit);
int get_pwm_freq(void);




static bemf_anal_t bemf_anal[MAX_CANTON_FOR_DETECTION] = {0};




// ---------------------------------------------

static int setfreqi(int fi)
{
	if (fi >= DETECT_NUM_FREQS) return -1;
	set_pwm_freq(freqs[fi], 1);
	return 0;
}

void detect2_init(void)
{
    detect_state = 0;
    detect_canton.v = 0xFF;
    detect_ltick = 0;
    save_freq = get_pwm_freq();
    freqindex = 0;
    setfreqi(freqindex);
    memset(bemf_anal, 0, sizeof(bemf_anal));
}

// ---------------------------------------------


static inline _UNUSED_ int32_t divp(int32_t a, int32_t b)
{
	if (!b) return 0;
	return a/b;
}
static void analyse_bemf(int cnum, int frequi)
{
    if (!dst.voff.count) {
        itm_debug1(DBG_DETECT|DBG_ERR, "no bemf", detect_canton.v);
        return;
    }
    int freq = get_pwm_freq();
    int rf = dst.voff.count * 1000 / MEAS_DURATION;
    itm_debug1(DBG_DETECT, "Z canton", detect_canton.v);
    itm_debug3(DBG_DETECT, "Z freq", freq, rf, dst.voff.count);
    itm_debug3(DBG_DETECT, "Z bemf", dst.voff.min, measval_avg(&dst.voff), dst.voff.max);
    itm_debug3(DBG_DETECT, "Z von",  dst.von.min, measval_avg(&dst.von), dst.von.max);
    itm_debug3(DBG_DETECT, "Z ina",  dst.ina.min, measval_avg(&dst.ina), dst.ina.max);


    bemf_anal_t *p = &bemf_anal[cnum];
    int32_t v =  measval_avg(&dst.voff); //*1000;

    p->R[frequi*2] = v;
    p->R[frequi*2+1] = measval_avg(&dst.ina);

    // U=RI, R=U/I -> 1/R
    //p->R[frequi] = divp(measval_avg(&dst.ina)*1000, measval_avg(&dst.von));

    /*
    if (frequi==0) {
    	if (p->R[0] > 100) {
    		itm_debug1(DBG_DETECT, "*ROKUHAN", cnum);
    		// Rokuhan chassis will start moving at lowest voltage thus having bemf>0
    		// we cant continue detection
    		p->d = loco_rokuhan_chassis;
    	}
    }
    if (frequi==1) {
    	if ((abs(bemf_anal[cnum].R[0]) < 12) && (abs(bemf_anal[cnum].R[1]) < 12)) {
    		// empty
    		itm_debug1(DBG_DETECT, "*empty", cnum);
    		p->d = loco_none;
    	}
    }
    */

}

void analyse_bemf_final(void)
{
    // clear all presence
    occupency_clear();
    
    // check for train
	itm_debug1(DBG_CTRL, "job here", 0);
	for (int cnum = 0; cnum < MAX_CANTON_FOR_DETECTION; cnum ++) {
		if (!bemf_anal[cnum].d) {
			char buf[12];

			itm_debug2(DBG_DETECT, "LOCO", cnum, bemf_anal[cnum].d);
			itm_write("/* C:", 5);
			itoa(cnum, buf, 10);
			itm_write(buf, (int)strlen(buf));
			itm_write("*/ {", 4);
			for (int fi = 0; fi < DETECT_NUM_FREQS*2; fi++) {
				if (fi)  itm_write(", ", 2);
				char buf[12];
				itoa(bemf_anal[cnum].R[fi], buf, 10);
				int l = MIN(12, (int) strlen(buf));
				itm_write(buf, l);
			}
			itm_write("}\n", 2);
			int16_t r = detect_loco_find(&bemf_anal[cnum]);
			bemf_anal[cnum].d = r;
		}
		itm_debug2(DBG_DETECT, ">>>", cnum, bemf_anal[cnum].d);
		const char *n = loco_detect_name(bemf_anal[cnum].d);
		itm_write("---->", 5); itm_write(n, (int) strlen(n)); itm_write("\n", 1);
        
        
        int altrnum = 0;  // for now trainnum are given in order
        if (bemf_anal[cnum].d != loco_none) {
        	xblkaddr_t bnum = {.v=cnum};
            lsblk_num_t sblk = any_lsblk_with_canton(bnum);
            // train num XXX TODO
            int trnum = altrnum;
            altrnum++;
            //...
            train_oldctrl_t *tvar = ctrl_get_tvar(trnum);
            ctrl2_init_train(trnum, tvar, sblk);
            ctrl2_set_mode(trnum, tvar, train_manual);
            
            // set occupency
        	//xblkaddr_t bn = {.v=cnum};
            set_block_addr_occupency(bnum, BLK_OCC_LOCO_STOP, trnum, sblk);
            
            // train params
            const conf_train_t *tconf = conf_train_get(trnum);
            conf_train_t *tconfm = (conf_train_t *)tconf;
            const conf_train_t *template = detect_loco_conf(bemf_anal[cnum].d);
            memcpy(tconfm, template, sizeof(*tconfm));
        }
	}
}

void detect2_process_tick(uint32_t tick)
{
	if (tick<6000) return;

	if (0) {
	} else if (-1 == detect_state) {
		return;
	} else if (0 == detect_state) {
        detect_canton.v++;
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
		        m.to = MA3_UI_GEN; //(UISUB_TFT);
		        mqf_write_from_ctrl(&m);

		        if ((1)) {
		        	//osDelay(500);
		        	msg_64_t m;
		        	m.from = MA3_BROADCAST;
		        	m.to = MA3_BROADCAST;
		        	m.cmd = CMD_SETRUN_MODE;
		        	m.v1u = runmode_normal;

		        	mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl
		        }

		        return;
            }

            itm_debug3(DBG_DETECT, "*freq", freqindex, freqs[freqindex], get_pwm_freq());
            detect_canton.v = 0;

        }
        if (bemf_anal[detect_canton.v].d) {
        	itm_debug2(DBG_DETECT, "already", detect_canton.v, bemf_anal[detect_canton.v].d);
            detect_state = 0;
			return;
        }

        // start detection on new canton
        detect_ltick = tick;
        itm_debug1(DBG_DETECT, "DETECT", detect_canton.v);

        if ((USE_INA_FOR_DETECT)) {
        	// start monitoring current (INA3221) on concerned sub blocks
        	uint16_t inas = get_ina_bitfield_for_canton(detect_canton.v);
            msg_64_t m = {0};
            m.from = MA1_CONTROL();
            m.to = MA0_INA(oam_localBoardNum()); // XXX board to be added
            m.cmd = CMD_START_INA_MONITOR;
            m.v1u = inas;
            mqf_write_from_ctrl(&m);
        }

        // start monitoring BEMF
        msg_64_t m = {0};
        m.from = MA1_CONTROL();
        TO_CANTON(m, detect_canton);
        //m.to = detect_canton; //MA_CANTON(0, canton);
        m.cmd = CMD_START_DETECT_TRAIN;
        m.v1u = 20; //%pwm
        mqf_write_from_ctrl(&m);

        // notify UI
        m.cmd = CMD_UI_DETECT;
        m.v1 = detect_canton.v;
        m.v2u = get_pwm_freq();;
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        mqf_write_from_ctrl(&m);

        detect_state = 1;
        memset(&dst, 0, sizeof(dst));
    }  else if (detect_state==1) {
        if (tick >= detect_ltick+MEAS_DURATION) {
        	// stop monitoring BEMF
            msg_64_t m = {0};
            TO_CANTON(m, detect_canton);
            //m.to = detect_canton; //MA_CANTON(0, canton);
            m.from = MA1_CONTROL();
            m.cmd = CMD_STOP_DETECT_TRAIN;
            mqf_write_from_ctrl(&m);
            itm_debug2(DBG_DETECT, "END", detect_canton.v, detect_ltick);
            detect_ltick = tick;
            detect_state = 2;

            if ((USE_INA_FOR_DETECT)) {
            	// stop monitoring current (INA3221)
            	msg_64_t m = {0};
            	m.from = MA1_CONTROL();
            	m.to = MA0_INA(oam_localBoardNum()); // XXX board to be added
            	m.cmd = CMD_START_INA_MONITOR;
            	m.v1u = 0;
            	mqf_write_from_ctrl(&m);
            }
            // analyse results
            analyse_bemf(detect_canton.v, freqindex);
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
    /*if (!IS_CANTON(m->from) ) {MA_GET_CANTON_NUM
    	return;
    } */
    switch (m->cmd) {
        case CMD_BEMF_NOTIF:
        	//itm_debug2(DBG_DETECT, "bemf", m->v1, m->v2); //XXX
            if (detect_state != 1) {
                itm_debug2(DBG_DETECT, "bad state", detect_state, m->from);
                break;
            }
            xblkaddr_t fc = FROM_CANTON(*m);
            if (fc.v != detect_canton.v) {
                itm_debug3(DBG_DETECT, "bad from", detect_canton.v, m->from, fc.v);
                break;
            }
            measval_addvalue(&dst.voff, m->v1);
            measval_addvalue(&dst.von, m->v2);
            if ((1)) {
            	if (!freqindex && (detect_canton.v == 0) && (dst.voff.count > 20) && (m->v1 > 2000)) {
            		extern volatile int oscillo_trigger_start;
            		oscillo_trigger_start = 1;
            	}
            }

            break;
        case CMD_INA_REPORT:
#if USE_INA_FOR_DETECT
        	if (detect_state != 1) {
        		itm_debug2(DBG_DETECT, "bad stat", detect_state, m->from);
        		break;
        	}
        	// XXX assume only one INA here
        	measval_addvalue(&dst.ina, m->v1);
        	if ((0)) itm_debug2(DBG_DETECT, "current", m->subc, m->v1);
        	break;
#endif
        default:
            break;
    }
}

/*
 * after 3 detect :
 *
0421486@q/0/3/255 CMD_SETRUN_MODE, to_canbus
0421486@q/1/3/128
0421486@q/2/3/255
0421486@q/3/3/128
0421486@q/4/3/255
0421487@q/5/3/128
0421487@q/6/3/255
 */

#else  // unmaintained

// -----------------------------------------------------------------


#include "train_detectors.h"

typedef  enum {
    state_finished          = -1,
    state_next_detector     = 0,
    state_next_canton       = 1,
    state_next_step         = 2,
    state_wait_on           = 3,
    state_next_stop_step    = 4,
} detector_state_t;


static detector_state_t detect_state = state_next_detector;
static xblkaddr_t detect_canton = {.v=0xFF};
static uint32_t detect_tick = 0;
static const train_detector_t *detector = NULL;
static const train_detector_step_t *detectorstep = NULL;

static int save_freq = 0;
static int freqindex = 0;


extern void set_pwm_freq(int freqhz, int crit);
extern int get_pwm_freq(void);


void detect2_init(void)
{
    detect_state = state_next_detector;
    detect_canton.v = 0xFF;
    //detect_ltick = 0;
    save_freq = get_pwm_freq();
}



static void _detection_finished(void)
{
    
    msg_64_t m = {0};

    m.cmd = CMD_UI_DETECT;
    m.v1 = -1;
    m.v2 = 0;
    m.to = MA3_UI_GEN; //(UISUB_TFT);
    mqf_write_from_ctrl(&m);

    if ((1)) {
        //osDelay(500);
        msg_64_t m;
        m.from = MA3_BROADCAST;
        m.to = MA3_BROADCAST;
        m.cmd = CMD_SETRUN_MODE;
        m.v1u = runmode_normal;

        mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl
    }
}

#define DELAY_INITIAL 100

void detect2_process_tick(_UNUSED_ uint32_t tick,  _UNUSED_ uint32_t dt)
{
    if (tick<6000) return;

    switch (detect_state) {
        case state_finished:
            break;
        default:
            //TODO
            FatalError("DEst", "bad detect state", Error_Other);
            detect_state = state_finished;
            _detection_finished();
            break;
        case state_next_detector:
            // starting / next detector
            if (!detector) {
                detector = &alldetectors;
            } else {
                detector = detector->next;
            }
            if (!detector) {
                // all done
                itm_debug1(DBG_DETECT, "NEXT DONE", 0);
                detect_state = state_finished;
                _detection_finished();
                break;
            }
            itm_debug1(DBG_DETECT, "NEXT DET", 0);
            itm_debug1(DBG_DETECT, detector->name, 0);
            detectorstep = NULL;
            detect_canton.v = 0xFF;
            detect_state = state_next_canton;
            detect_tick = tick;
            break;

            
        case state_next_canton:
            detect_canton.v++;
            if (0xFF == detect_canton.v) {
                detect_state = state_next_detector;
                break;
            }
            itm_debug1(DBG_DETECT, "next C", detect_canton.v);
            detectorstep = NULL;
            detect_state = state_next_step;
            break;
            
        case state_next_step: // FALLTHRU
        case state_next_stop_step:
            // start canton steps
            if (!detectorstep) {
                detectorstep = detector->steps;
            } else {
                detectorstep = detectorstep->nextstep;
            }
            if (!detectorstep && (detect_state == state_next_step)) {
                detect_state = state_wait_on;
                detect_tick = tick;
                break;
            }
            if (!detectorstep) {
                // all step done
                detect_state = state_next_canton;
                break;
            }
            int rc = 0;
            if (detect_state == state_next_step) {
                if (detectorstep->detect_start_canton) {
                    rc = detectorstep->detect_start_canton(detect_canton);
                }
            } else {
                if (detectorstep->detect_stop_canton) {
                    rc = detectorstep->detect_stop_canton(detect_canton);
                }
            }
            if (rc<0) {
                detect_state = state_next_canton;
                break;
            }
            break;
            
        case state_wait_on:
            if (tick>=detect_tick+500) {
                detect_state = state_next_stop_step;
            }
            break;
    }
}
    

void detect2_process_msg(_UNUSED_ msg_64_t *m)
{
    switch (m->cmd) {
        case CMD_BEMF_NOTIF:
            break;
            
        case CMD_DETECTION_REPORT:
            switch (detect_state) {
                case state_wait_on:
                case state_next_step:
                    itm_debug1(DBG_DETECT, "FOUND", m->subc);
                    break;
                    
                default: //ignore report
                    itm_debug1(DBG_DETECT, "ignore report", m->subc);
                    break;
            }
            break;
        default:
            itm_debug1(DBG_DETECT, "unhmsg", m->cmd);
            break;
    }
}

#endif // unmaintained
