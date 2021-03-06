/*
 * disp.h
 *
 *  Created on: May 10, 2021
 *      Author: danielbraun
 */

#ifndef IHM_DISP_H_
#define IHM_DISP_H_

void _test_new_disp(void);

#define MAX_DISP 1
void disp_layout(int numdisp);

/*
 * layouts
 * 0 = default (version/copyright)
 * 1 = manual drive (speed/bar)
 * 200 = ina3221 i2c detection
 */
#define LAYOUT_DEFAULT	0
#define LAYOUT_MANUAL	1
#define LAYOUT_INA3221_DETECT	200
#define	LAYOUT_INA3221_VAL		201

void ihm_setlayout(int numdisp, int numlayout);

#define DISP_MAX_REGS 16
void ihm_setvar(int numdisp, int varnum, uint16_t val);
uint16_t ihm_getvar(int numdisp, int varnum);

#endif /* IHM_DISP_H_ */
