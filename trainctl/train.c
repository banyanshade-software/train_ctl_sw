/*
 * train.c
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

#include "misc.h"
#include "train.h"
#include "railconfig.h"

void train_reset(const train_config_t *c, train_vars_t *v)
{
	/*
	 *
typedef struct train_vars {
	pidctl_vars_t pidvars;
	inertia_vars_t inertiavars;
	uint32_t lasttick;

	int16_t current_canton;
	int16_t next_canton;
	int8_t  current_canton_dir; // -1 or +1
	int8_t  next_canton_dir;

	uint16_t target_speed;

	uint16_t last_speed;
	uint16_t cur_c1_volt_idx;
	uint16_t cur_c2_volt_idx;
} train_vars_t;
	 */
	pidctl_reset(&c->pidcnf, &v->pidvars);
	inertia_reset(&c->inertiacnf, &v->inertiavars);
	//v->lasttick = 0;
	v->current_canton = 0; // XXX
	v->next_canton = 0;	   // XXX
	v->current_canton_dir = 1;
	v->next_canton_dir = 1;
	v->target_speed = 0;
	v->cur_c1_volt_idx = 0xFFFF;
	v->cur_c2_volt_idx = 0xFFFF;

	v->position_estimate = 0;
}

int train_reset_pos_estimate(int tidx)
{
	train_vars_t *v = get_train_vars(tidx);
	if (!v) return train_error(ERR_BAD_PARAM, "bad train num");
	v->position_estimate = 0;
	return 0;
}

