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
void setvar(int numdisp, int varnum, uint16_t val);

#endif /* IHM_DISP_H_ */
