//
//  train_detectors.h
//  train_throttle
//
//  Created by Daniel Braun on 09/07/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef train_detectors_h
#define train_detectors_h

#include "misc.h"
#include "../msg/trainmsg.h"

typedef struct {
    xblkaddr_t canton;
    ina_num_t ina;
    lsblk_num_t lsblk;
    uint8_t numlsblk;   // detector will know canton+ina, which correspond
                        // to one or several lsblk
    uint8_t locotype;   // id of locomotive, 0xFF = unknown

    // no possibility for now to detect train length
    //uint8_t leftcm;
    //uint8_t rightcm;
    
} train_detector_result_t;

const train_detector_result_t *detector_result_for_canton(xblkaddr_t c);

/*
 we have several detectors, that will be run sequencially
 The first detector typically detect train presence (by applying a short
 pwm power, and measuring consumed current through ina3221.
 The next detectors will detect trains on lsblk that may not be moninoted by ina3221
 (no info on current consumtion) and try to recognize locomotive type,
 either by measuring frequency response or by measuring impulse response.
 
 For each canton, each detector will run a sequence of start steps, and,
 spcifically if detection is interrupted, a sequence of stop steps
 
 Each step returns normally 0, and will return -1 if steps should be interrupted
 for this canton.

 wait state may return > 0 : low 16 bits to indicate a sleep in ms
                             0x10000 indicate wait stop on first report
 */

#define RC_DELAY(_ms) (_ms)
#define RC_DELAY_R(_ms) (0x10000 | (_ms))

typedef struct st_detector_step {
    const struct st_detector_step *nextstep;
    int (*detect_start_canton)(xblkaddr_t);
    int (*detect_stop_canton)(xblkaddr_t);
} train_detector_step_t;

typedef struct st_detector {
    // use lisp like cons instead to have const struct
	// const struct st_detector *next;
    // init and deinit called when starting/ending detector
    void (*const detect_init)(uint8_t p); // parameter unused, 0
    void (*const detect_deinit)(void);
    // parse handles CMD_DETECTION_REPORT
    int (*const detect_parse)(const msg_64_t *m, train_detector_result_t *d, xblkaddr_t canton);
    // ...
    const train_detector_step_t *steps;
    const char *name; // for debug
} train_detector_t;

typedef struct detect_cons {
	const train_detector_t *d;
	const struct detect_cons *next;
} train_detect_cons_t;




/* detectors */
extern const train_detect_cons_t alldetectors;

#endif /* train_detector_h */
