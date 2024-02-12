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

#include "../ctrl/ctrl.h"
#include "../ctrl/locomotives.h"

#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

#include "detect.h"
#include "train_detectors_params.h"

//#define MEAS_DURATION  300 //tick = ms
//#define RELAX_DURATION 200


// -----------------------------------------------------------------


#include "train_detectors.h"

typedef  enum {
    state_finished          = 0,
    state_start             = 1,
    state_next_detector     = 2,
    state_next_canton       = 3,
    state_next_step         = 4,
    state_wait_on           = 5,
    state_next_stop_step    = 6,
    state_wait_report       = 7,
} __attribute((packed)) detector_state_t;


static detector_state_t detect_state = state_finished;
static detector_state_t saved_state = state_finished;
static xblkaddr_t detect_canton = {.v=0xFF};
static uint32_t detect_tick = 0;
static uint32_t waketick = 0;
static uint8_t gotreport = 0;
static uint8_t waitreport = 0;
static const train_detect_cons_t *detector = NULL;
static const train_detector_step_t *detectorstep = NULL;

static int save_freq = 0;
//static int freqindex = 0;


//extern void set_pwm_freq(int freqhz, int crit);
extern int get_pwm_freq(void);

#define MAX_DETECT_TRAINS 4
train_detector_result_t result[MAX_DETECT_TRAINS] = {0};

void detect2_start(void)
{
    detect_state = state_start; //state_next_detector;
    detect_canton.v = 0xFF;
    detect_tick = HAL_GetTick();
    for (int i=0; i<MAX_DETECT_TRAINS; i++) {
        result[i].canton.v = 0xFF;
        result[i].lsblk.n = -1;
        result[i].numlsblk = 0;
        result[i].locotype = 0xFF;
        result[i].ina.v    = 0xFF;
    }
    //detect_ltick = 0;
    save_freq = get_pwm_freq();
}

static void send_oam_none(void);
static void send_oam_report(train_detector_result_t *r, int last);

static void _detection_finished(void)
{
    
	for (int i=0; i<MAX_DETECT_TRAINS; i++) {
		if (result[i].canton.v == 0xFF) continue;
		itm_debug3(DBG_DETECT, "Rdet", result[i].lsblk.n, result[i].canton.v, result[i].locotype);
	}
#ifdef TRAIN_SIMU
    /* set fixed repport values for O&M tests*/
    result[0].locotype = Marklin8821_V200;
    result[0].canton.v = 2;
    result[0].ina.v = 0xF;
    result[0].lsblk.n = 5;
    result[0].numlsblk = 1;

    result[1].locotype = Marklin8895_BR74;
    result[1].canton.v = 4;
    result[1].ina.v = 0xF;
    result[1].lsblk.n = 8;
    result[1].numlsblk = 2;

#endif
    for (int i=0; i<MAX_DETECT_TRAINS; i++) {
        int last = 0;
        if ((i==MAX_DETECT_TRAINS-1) || (result[i+1].canton.v == 0xFF)) {
            last = 1; // this is last report
        }
        if (last && (0xFF==result[i].canton.v)) {
            // none found
            send_oam_none();
            break;
        }
        send_oam_report(&result[i], last);
        if (last) break;
    }

    // notify UI
    msg_64_t m = {0};

    m.cmd = CMD_UI_DETECT;
    m.v1 = -1;
    m.v2 = 0;
    m.to = MA3_UI_GEN; //(UISUB_TFT);
    mqf_write_from_ctrl(&m);

    // moved to oam
    if ((0)) {
        //osDelay(500);
        msg_64_t md;
        md.from = MA3_BROADCAST;
        md.to = MA3_BROADCAST;
        md.cmd = CMD_SETRUN_MODE;
        md.v1u = runmode_normal;

        mqf_write_from_nowhere(&md);
    }
}


static int detect_step_notify_ui(int numdetector, xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "D-gui", detect_canton.v);
    if ((1)) {
        msg_64_t m = {0};
        
        m.cmd = CMD_UI_DETECT;
        m.v1 = detect_canton.v;
        m.v2 = numdetector;
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        m.from = MA1_CONTROL();
        mqf_write_from_ctrl(&m);
    }
    return 0;
}

#define DELAY_INITIAL 100

static int num_detector = 0; // for UI notification

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
        case state_start:
            detector = NULL;
            num_detector = 0;
            if (tick >= detect_tick+500) {
                debug_info('X', 0, "Start detect", 0,0,0);
            	detect_state = state_next_detector;
            }
            break;
        case state_next_detector:
            // starting / next detector
            if (detector && detector->d->detect_deinit) {
                detector->d->detect_deinit();
            }
            if (!detector) {
                detector = &alldetectors;
                num_detector = 0;
            } else {
                detector = detector->next;
                num_detector++;
            }
            if (!detector) {
                // all done
                itm_debug1(DBG_DETECT, "NEXT DONE", 0);
                detect_state = state_finished;
                _detection_finished();
                break;
            }
            itm_debug1(DBG_DETECT, "NEXT DET", 0);
            itm_debug1(DBG_DETECT, detector->d->name, 0);
            if (detector->d->detect_init) {
                detector->d->detect_init(0);
            }
            detectorstep = NULL;
            detect_canton.v = 0xFF;
            detect_state = state_next_canton;
            detect_tick = tick;
            break;

            
        case state_next_canton:
        	for (;;) {
        		detect_canton.v++;
        		gotreport = 0;
        		if (0xFF == detect_canton.v) {
        			detect_state = state_next_detector;
        			break;
        		}
        		// check if canton exists:
        	    lsblk_num_t lsb = any_lsblk_with_canton(detect_canton);
        	    if (lsb.n<0) {
                    itm_debug1(DBG_DETECT, "skip C", detect_canton.v);
        	    	continue;
        	    }
        	    break;
        	}
        	if (0xFF == detect_canton.v) break;

            itm_debug1(DBG_DETECT, "next C", detect_canton.v);
            detectorstep = NULL;
            detect_state = state_next_step;
            break;
            
        case state_next_step: // FALLTHRU
        case state_next_stop_step:
            // start canton steps
            if (!detectorstep) {
                detectorstep = detector->d->steps;
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
                    detect_step_notify_ui(num_detector, detect_canton);
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
            } else if ((rc>0) && !gotreport) {
            	saved_state = detect_state;
            	waketick = tick+(rc & 0xFFFF);
            	waitreport = rc & (0x10000) ? 1 : 0;
            	detect_state = state_wait_report;
            }
            break;

        case state_wait_report:
        	if (tick > waketick) {
        		itm_debug1(DBG_DETECT|DBG_ERR, "rep timeout", 0);
        	}
        	if ((waitreport && gotreport) || (tick > waketick)) {
        		itm_debug1(DBG_DETECT, "E-wait", gotreport);
        		detect_state = saved_state;
        		waketick = 0;
        	}
        	break;
        case state_wait_on:
            if (tick>=detect_tick+500) {
                detect_state = state_next_stop_step;
            }
            break;
    }
}
    
static void register_found(train_detector_result_t *res)
{
    debug_info('X', 0, "found presence", res->canton.v, res->lsblk.n, res->locotype);
    for (int i=0; i<MAX_DETECT_TRAINS; i++) {
        if (result[i].canton.v == 0xFF) {
            memcpy(&result[i], res, sizeof(*res));
            return;
        }
        if (result[i].canton.v == res->canton.v) {
            if (res->lsblk.n != -1) {
                if (result[i].lsblk.n == -1) {
                    result[i].lsblk = res->lsblk;
                } else if (result[i].lsblk.n !=  res->lsblk.n) {
                    result[i].lsblk = res->lsblk; // yes ?
                }
            }
            if (res->numlsblk>0) {
                if (result[i].numlsblk > res->numlsblk) {
                    result[i].numlsblk = res->numlsblk;
                }
            }
            if (res->locotype != 0xFF) {
                result[i].locotype = res->locotype;
            }
            if (res->ina.v != 0xFF) {
            	result[i].ina = res->ina;
            }
            return;
        }
    }
    // too many trains detected
    itm_debug1(DBG_DETECT|DBG_ERR, "det-many", MAX_DETECT_TRAINS);
}

const train_detector_result_t *detector_result_for_canton(xblkaddr_t c)
{
#ifdef _TEST_FREQ_ON_S5
	//// ---- for test with freq detector only
	if ((1)) {
		if (c.v==2) {
			result[0].canton.v = 2;
			result[0].ina.v = 7;
			result[0].lsblk.n = 5;
			result[0].locotype = 0xFF;
			return &result[0];
		}
	}
	//// ----
#endif // _TEST_FREQ_ON_S5
    for (int i=0; i<MAX_DETECT_TRAINS; i++) {
        if (result[i].canton.v == 0xFF) {
        	return NULL;
        }
        if (result[i].canton.v == c.v) return &result[i];
    }
    return NULL;
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
                case state_wait_report:
                    itm_debug1(DBG_DETECT, "FOUND", m->subc);
                    gotreport = 1;
                    train_detector_result_t res = {0};
                    int rc = detector->d->detect_parse(m, &res, detect_canton);
                    if (rc) {
                        itm_debug2(DBG_DETECT|DBG_ERR, "dtprse", m->from, m->subc);
                        break;
                    }
                    itm_debug2(DBG_DETECT, "--->", m->subc, m->vb0);
                    register_found(&res);
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

/* OAM reporting */


static void send_oam_none(void)
{
    msg_64_t m = {0};
    m.from = MA3_BROADCAST; // XXX
    m.to = MA0_OAM(0);
    m.cmd = CMD_OAM_DETECTREPORT;
    m.subc = 0xFF;
    m.vb0 = 0xFF;
    m.vb1 = 0xFF;
    m.vb2 = 0;
    m.vb3 = 0;
    
    mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl
}

static void send_oam_report(train_detector_result_t *r, int last)
{
    msg_64_t m = {0};
    m.from = MA3_BROADCAST; // XXX
    m.to = MA0_OAM(0);
    m.cmd = CMD_OAM_DETECTREPORT;
    m.subc = last;
    m.vb0 = r->canton.v;
    m.vb1 = r->lsblk.n;
    m.vb2 = r->locotype;
    m.vb3 = r->numlsblk;
    
    mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl

}
