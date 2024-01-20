//
//  train_detectors.c
//  train_throttle
//
//  Created by Daniel Braun on 10/07/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//


#include "misc.h"
#include "../msg/trainmsg.h"
#include "../topology/topology.h"
#include "train_detectors.h"
#include "train_detectors_params.h"
#include "train_detectors_p.h"
#include "train_detector_freqresp.h"


/* parser */
static _UNUSED_ int nil_detect_parse(_UNUSED_ const msg_64_t *m, train_detector_result_t *r)
{
    memset(r, 0, sizeof(*r));
    return 0;
}

static int detect_ina_parser(const msg_64_t *m,  train_detector_result_t *r)
{
    if (!MA0_IS_INA(m->from)) {
        return -1;
    }
    _UNUSED_ uint8_t brd = MA0_BOARD(m->from);
    ina_num_t ina;
    ina.ina = m->subc;
    ina.board = brd;
    get_lsblk_and_canton_for_ina(ina, &(r->lsblk), &(r->canton));
    if (r->canton.v == 0xFF) {
        memset(r, 0, sizeof(*r));
        return -1;
    }
    r->ina = ina;
    r->locotype = 0xFF;
    r->numlsblk = 1;
    lsblk_num_t n = r->lsblk;
    for (;;) {
        n = get_nextlsblk_with_same_ina(n);
        if (n.n == -1) break;
        r->numlsblk ++;
    }
    return 0;
}
/* start actions step */
int detect_step_check_canton_exist(xblkaddr_t detect_canton)
{
    lsblk_num_t lsb = any_lsblk_with_canton(detect_canton);
    if (lsb.n<0) {
        return  -1;
    }
    return 0;
}

int detect_step_notify_ui(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "D-gui", detect_canton.v);
    if ((1)) {
        msg_64_t m = {0};
        
        m.cmd = CMD_UI_DETECT;
        m.v1 = detect_canton.v;
        m.v2 = 0;
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        mqf_write_from_ctrl(&m);
    }
    return 0;
}

static void _start_canton(xblkaddr_t detect_canton, uint8_t method, uint16_t param)
{
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    TO_CANTON(m, detect_canton);
    m.cmd = CMD_START_DETECT_TRAIN;
    m.v2u = method;
    m.v1u = param; //%pwm
    mqf_write_from_ctrl(&m);
}
int detect_step_start_pwm(xblkaddr_t  detect_canton)
{
    itm_debug1(DBG_DETECT, "D-pwm", detect_canton.v);
    _start_canton(detect_canton, 1, 20);
    return 0;
}
int detect_step_wait(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "D-wait", detect_canton.v);
    return 0;
}
static int detect_step_start_inameas(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "D-ina", detect_canton.v);

    uint16_t inas = get_ina_bitfield_for_canton(detect_canton.v);
    if (!inas) return -1;
    int board = detect_canton.board;
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA0_INA(board);
    m.cmd = CMD_START_INA_MONITOR;
    m.va16 = inas;
    m.vb8 = 1; // mode std

    mqf_write_from_ctrl(&m);
    
    return 0;
}



/* stop actions steps */
int detect_step_stop_pwm(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "O-pwm", detect_canton.v);
    msg_64_t m = {0};
    TO_CANTON(m, detect_canton);
    //m.to = detect_canton; //MA_CANTON(0, canton);
    m.from = MA1_CONTROL();
    m.cmd = CMD_STOP_DETECT_TRAIN;
    mqf_write_from_ctrl(&m);
    return 0;
}

static _UNUSED_ int detect_step_stop_inameas(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "O-ina", detect_canton.v);
    
    int board = detect_canton.board;
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA0_INA(board);
    m.cmd = CMD_START_INA_MONITOR;
    m.va16 = 0; // bitfield zero to stop all monitoring
    m.vb8 = 0;
    mqf_write_from_ctrl(&m);
    
    return 0;
}


int detect_step_stop_notify_ui(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "O-gui", detect_canton.v);
    if ((1)) {
        msg_64_t m = {0};
        
        m.cmd = CMD_UI_DETECT;
        m.v1 = -1;//detect_canton.v;
        m.v2 = 0;
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        mqf_write_from_ctrl(&m);
    }
    return 0;
}


/* ------------------------------------------------------  */


static const train_detector_step_t _detector1_step3 = {
    .detect_start_canton = detect_step_wait,
    .detect_stop_canton = NULL,
    .nextstep = NULL
};

static const train_detector_step_t _detector1_step2 = {
    .detect_start_canton = detect_step_start_pwm,
    .detect_stop_canton = detect_step_stop_pwm,
    .nextstep = &_detector1_step3
};

static const train_detector_step_t _detector1_step1 = {
    .detect_start_canton = detect_step_start_inameas,
    .detect_stop_canton = detect_step_stop_inameas,
    .nextstep = &_detector1_step2
};

static const train_detector_step_t _detector1_step0 = {
    .detect_start_canton = detect_step_check_canton_exist,
    .detect_stop_canton = NULL,
    .nextstep = &_detector1_step1
    
};

/* ---------------------------------------------------------- */
/* ---------------------------------------------------------- */
/* ---------------------------------------------------------- */
/* ---------------------------------------------------------- */

const train_detector_t normal_detector = {
    .detect_init = NULL,
    .detect_deinit = NULL,
    .detect_parse = detect_ina_parser,
    .steps = &_detector1_step0
};


#ifdef TRAIN_SIMU

static void detect_simu_init(uint8_t p)
{
    itm_debug1(DBG_DETECT, "+++ SIMU", p);
}
static void detect_simu_deinit(void)
{
    itm_debug1(DBG_DETECT, "--- SIMU", 0);
}

static int detect_simu0(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "DETECT0", detect_canton.v);
    return detect_step_check_canton_exist(detect_canton);
}

static int detect_simu1(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "DETECT1", detect_canton.v);
    return 0;
}

static int detect_esimu0(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "END0", detect_canton.v);
    return detect_step_check_canton_exist(detect_canton);
}

static int detect_esimu1(xblkaddr_t detect_canton)
{
    itm_debug1(DBG_DETECT, "END1", detect_canton.v);
    return 0;
}


static const train_detector_step_t _simu_step1 = {
    .detect_start_canton = detect_simu1,
    .detect_stop_canton = detect_esimu1,
    .nextstep = NULL,
};
static const train_detector_step_t _simu_step0 = {
    .detect_start_canton = detect_simu0,
    .detect_stop_canton = detect_esimu0,
    .nextstep = &_simu_step1
};

static const train_detector_t detect1 = {
    .detect_init = NULL,
    .detect_deinit = NULL,
    .detect_parse = detect_ina_parser,
    .steps = &_detector1_step0,
    .name = "INA_DET",
    
};

const train_detector_t simu_detector = {
    .detect_init = detect_simu_init,
    .detect_deinit = detect_simu_deinit,
    .detect_parse = nil_detect_parse,
    .steps = &_simu_step0,
    .name = "SIMU",
};

const train_detect_cons_t c1 = {
		.d = &normal_detector,
		.next = NULL,
};
const train_detect_cons_t alldetectors = {
		.d = &simu_detector,
		.next = &c1,
};
#else
#ifndef _TEST_FREQ_ON_S5

static const train_detect_cons_t d1 = {
		.d = &freqresp_detector,
		.next = NULL
};
const train_detect_cons_t alldetectors = {
		.d = &normal_detector,
		.next = &d1
};
#else // _TEST_FREQ_ON_S5
const train_detect_cons_t alldetectors = {
		.d = &freqresp_detector,
		.next = NULL
};
#endif // _TEST_FREQ_ON_S5
#endif


