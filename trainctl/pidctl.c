/*
 * pidctl.c
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


/* pidctl.c : implementation of PID (Proportional Integral Derivative) speed control
 * 			  feedback is provided by BEMF measured during "off" (high impedance) time of PWM
 * 			  PWM duty cycle is limited to 90% (thus max power is slightly limited) for this
 * 			  note that kD parameter is internaly divided by 1000
 *
 * 			  pidctl itself is generic, and operates on BEMF measured in centivolt (0.01V)
 *
 */


#include <stdint.h>
#include <stddef.h>
#include <memory.h>
#include "pidctl.h"
#include "misc.h"

void pidctl_reset(const pidctl_config_t *c, pidctl_vars_t *v)
{
    v->has_last = 0;
	v->last_err = 0;
	v->sume     = 0;
	v->target_v = 0;
	v->stopped = 1;
	v->has_last = 0;
}

void pidctl_set_target(const pidctl_config_t *c, pidctl_vars_t *v, int32_t val)
{
	if ((0)) pidctl_reset(c,v);
    if ((0)) v->sume = val;
	v->target_v = val;
}

#define MAX_I (25000)

int32_t pidctl_value(const pidctl_config_t *c, pidctl_vars_t *v, int32_t cur_v, uint32_t dt)
{
	// cuv in native BEMF value
    if (!dt) dt = 1;
    if (dt>100) dt=100;
	int32_t err = v->target_v - cur_v;
    if (err> 2*MAX_PID_VALUE) err =  2*MAX_PID_VALUE;
    if (err<-2*MAX_PID_VALUE) err = -2*MAX_PID_VALUE;

    int32_t dv = (v->has_last) ? 1000*(err - v->last_err)/((int32_t)dt) : 0; //XXX
	v->last_err = err;
    v->has_last = 1;
	if ((1)) v->sume += err*dt;
    else     v->sume = v->sume*.99 + err*dt;
    if (v->sume>MAX_I) v->sume = MAX_I;
    else if (v->sume<-MAX_I) v->sume = -MAX_I;
	int32_t iv = v->sume / 100;

	//debug_info('T', 0, "PID  ", err, iv, dv);
	itm_debug2("pid tc", v->target_v, cur_v);
    itm_debug3("pid edi", err, dv, v->sume);

	int32_t r = c->kP * err + (c->kD * dv)/1000 + c->kI * iv;
	//debug_info('T', 0, "PID*k",  c->kP * err, c->kI * iv, (c->kD * dv)/1000);

	return r/1000;
}


