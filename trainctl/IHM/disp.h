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
 * layouts (predefined screens)
 * 0 = default (version/copyright)
 * 1 = manual drive (speed/bar)
 * 200 = ina3221 i2c detection
 */
#define LAYOUT_DEFAULT	0
#define LAYOUT_INIT		1	// shortly displayed at init
#define LAYOUT_OFF		2	// diag/UI: off runmode
#define LAYOUT_MANUAL	3	// UI : train in manual mode
#define LAYOUT_AUTO		4	// UI : train in auto mode
#define LAYOUT_NO_TRAIN 5	// UI : train is off / no train


#define LAYOUT_MASTER	8	// diag : master waiting for slave boards
#define LAYOUT_SLAVE	9	// diag : slave board waiting for master ack


#define LAYOUT_DETECT1	10	// to be updated, train detection run mode
#define LAYOUT_DETECT2	11	// to be updated, train detetction run mode
#define LAYOUT_TESTCAN  12	// diag : test CAN runmode
#define LAYOUT_TESTCANTON 13	// diag : test canton run mode

#define LAYOUT_INA3221_DETECT	20	// diag : ina3221 test run mode
#define	LAYOUT_INA3221_VAL		21	// diag : ina3221 test run mode

#define LAYOUT_FATAL	22	// diag : fatal error display


void ihm_setlayout(int numdisp, int numlayout);

#define DISP_MAX_REGS 16
void ihm_setvar(int numdisp, int varnum, uint16_t val);
uint16_t ihm_getvar(int numdisp, int varnum);

/* ihm vars for init/manual/auto :
0 = train num
1 = rot position
2 = dir+speed notified by ctrl
3 = 10+state
4 = dir (-1,0,+1)


ihm vars for detect1 :
5 = Von
6 = Voff (B)
7 = I
8 = rot/pwm
9 = vidx

*/
#endif /* IHM_DISP_H_ */
