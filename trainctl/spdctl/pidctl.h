/*
 * pidctl.h
 *
 *  Created on: Oct 3, 2020
 *      Author: danielbraun
 */



/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/* pidctl.h : implementation of PID (Proportional Integral Derivative) speed control
 * 			  feedback is provided by BEMF measured during "off" (high impedance) time of PWM
 * 			  PWM duty cycle is limited to 90% (thus max power is slightly limited) for this
 * 			  note that kD parameter is internaly divided by 1000
 *
 * 			  pidctl itself is generic, and operates on BEMF measured in centivolt (0.01V)
 */

#ifndef PIDCTL_H_
#define PIDCTL_H_

typedef struct pidctl_config {
	int32_t kP;
	int32_t kI;
	int32_t kD;
} pidctl_config_t;

typedef struct pidctl_vars {
	int32_t last_err;
	int32_t sume;
	int32_t target_v;
    uint8_t has_last:1;
    uint8_t stopped:1;
} pidctl_vars_t;

void pidctl_reset(const pidctl_config_t *c, pidctl_vars_t *v);

void pidctl_set_target(const pidctl_config_t *c, pidctl_vars_t *v, int32_t val);
int32_t pidctl_value(const pidctl_config_t *c, pidctl_vars_t *v, int32_t cur_v);

/*
 * PID ctrl operates on raw BEMF values, mmostly between -280..280 (-1V..+1V)
 */

// TODO update this, since we now operate on centivolt
#define MAX_PID_VALUE 3000

#endif /* PIDCTL_H_ */
