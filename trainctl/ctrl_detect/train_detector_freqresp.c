/*
 * train_detector_freqresp.c
 *
 *  Created on: Jul 31, 2023
 *      Author: danielbraun
 */



#include "misc.h"
#include "../msg/trainmsg.h"
#include "../topology/topology.h"
#include "train_detectors.h"
#include "train_detectors_p.h"


static _UNUSED_ int nil_detect_parse(_UNUSED_ const msg_64_t *m, train_detector_result_t *r)
{
    memset(r, 0, sizeof(*r));
    return 0;
}

static ina_num_t ina = {.v = 0xFF};

static void detect_freq_init(_UNUSED_ uint8_t p)
{
	ina.v = 0xFF;
}

static void detect_freq_deinit(void)
{
	ina.v = 0xFF;
}


static int detect_step_start_inafreqmeas(xblkaddr_t detect_canton)
{
	//return 0; // XXXX to test siggen
    itm_debug1(DBG_DETECT, "Df-ina", detect_canton.v);

	if (ina.v == 0xFF) return -1;

    int board = detect_canton.board;
    if (ina.board != board) return -1;

    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA0_INA(board);
    m.cmd = CMD_START_INA_MONITOR;
    m.va16 = ina.ina;
    m.vb8 = 2;

    mqf_write_from_ctrl(&m);

    return 0;
}

static  int detect_step_stop_inafreqmeas(xblkaddr_t detect_canton)
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

int detect_step_check_detection(xblkaddr_t detect_canton)
{
	//return 0; // XXXX to test siggen
	const train_detector_result_t *res = detector_result_for_canton(detect_canton);
	if (!res) return -1;
	if (res->canton.v == 0xFF) return -1;
	ina = res->ina;
	if (ina.v == 0xFF) return -1;
    return 0;
}

// ------------------------

int detect_dirac_start(xblkaddr_t detect_canton)
{
	msg_64_t m = {0};
	m.from = MA1_CONTROL();
	TO_CANTON(m, detect_canton);
	m.cmd = CMD_START_DETECT_TRAIN;
	m.v2u = 2; // special method
	m.v1u = 1; // dirac
	mqf_write_from_ctrl(&m);
	return 0;
}

int detect_dirac_stop(xblkaddr_t detect_canton)
{
	itm_debug1(DBG_DETECT, "O-pwm", detect_canton.v);
	msg_64_t m = {0};
	TO_CANTON(m, detect_canton);
	m.from = MA1_CONTROL();
	m.cmd = CMD_STOP_DETECT_TRAIN;
	mqf_write_from_ctrl(&m);
	return 0;
}


// ------------------------

static const train_detector_step_t _freq_step4 = {
    .detect_start_canton = detect_step_wait,
    .detect_stop_canton = NULL,
    .nextstep = NULL
};


static const train_detector_step_t _freq_step3 = {
		.detect_start_canton = detect_dirac_start, //detect_step_start_pwm,
		.detect_stop_canton = detect_dirac_stop, // detect_step_stop_pwm,
		.nextstep = &_freq_step4
};

static const train_detector_step_t _freq_step2 = {
    .detect_start_canton = detect_step_start_inafreqmeas,
    .detect_stop_canton = detect_step_stop_inafreqmeas,
    .nextstep = &_freq_step3
};

static const train_detector_step_t _freq_step1 = {
    .detect_start_canton = detect_step_check_detection,
    .detect_stop_canton = NULL,
    .nextstep = &_freq_step2
};

static const train_detector_step_t _freq_step0 = {
    .detect_start_canton = detect_step_check_canton_exist,
    .detect_stop_canton = NULL,
    .nextstep = &_freq_step1

};

// ------------------------

const train_detector_t freqresp_detector = {
		.detect_init = detect_freq_init,
		.detect_deinit = detect_freq_deinit,
		.detect_parse = nil_detect_parse,
		.steps = &_freq_step0,
		.name = "FreqResp",
};
