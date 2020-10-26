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
} pidctl_vars_t;

void pidctl_reset(const pidctl_config_t *c, pidctl_vars_t *v);

void pidctl_set_target(const pidctl_config_t *c, pidctl_vars_t *v, int32_t val);
int32_t pidctl_value(const pidctl_config_t *c, pidctl_vars_t *v, int32_t cur_v, uint32_t ellapsed_tick);

/*
 * PID ctrl operates on raw BEMF values, mmostly between -280..280 (-1V..+1V)
 */
#define MAX_PID_VALUE 280

#endif /* PIDCTL_H_ */
