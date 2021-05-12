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

void setlayout(int numdisp, int numlayout);
void ihm_setvar(int numdisp, int varnum, uint16_t val);
uint16_t ihm_getvar(int numdisp, int varnum);

#endif /* IHM_DISP_H_ */
