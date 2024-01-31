/*
 * detect_loco.h
 *
 *  Created on: Mar 9, 2022
 *      Author: danielbraun
 */

#ifndef CTRL_DETECT_LOCO_H_
#define CTRL_DETECT_LOCO_H_

#error obsolete

#define DETECT_NUM_FREQS 3

typedef struct {
	int16_t d;
	int16_t R[DETECT_NUM_FREQS*2];
} bemf_anal_t;


int16_t detect_loco_find(bemf_anal_t *d);

enum loco_detect_locos {
	loco_none = -1,
	loco_unknown = 0,
	loco_rokuhan_chassis,
	loco_8875_v160,		/* big red, reversed motor */
	loco_8821_v221,		/* big blue */
	loco_8805_br89,		/* mini steam black */
	loco_8864_v60,		/* small red one */
	loco_8864_v60b,		/* small red one, new one with red roof */
    
    loco_end

};

const char *loco_detect_name(enum loco_detect_locos l);

const conf_train_t *detect_loco_conf(enum loco_detect_locos loco);
#endif /* CTRL_DETECT_LOCO_H_ */
