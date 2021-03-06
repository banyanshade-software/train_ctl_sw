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


#include <stdint.h>
#include <stddef.h>
#include <memory.h>

#include "misc.h"
#include "train.h"
#include "railconfig.h"

#if 0
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
	memset(v, 0, sizeof(*v));
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
#endif


/* ------------------------------------------------------------------------------- */

const param_t train_params[] = {
		{ "kp",		 NULL, offsetof(train_config_t, pidcnf.kP), 		NULL,NULL, sizeof(int32_t), -3000, 3000,  150},
		{ "ki",		 NULL, offsetof(train_config_t, pidcnf.kI), 		NULL,NULL, sizeof(int32_t), -3000, 3000,  50},
		{ "kd",   	 NULL, offsetof(train_config_t, pidcnf.kD),			NULL,NULL, sizeof(int32_t), -3000, 3000, 550},
		{ "dec",   	 NULL, offsetof(train_config_t, inertiacnf.dec),    NULL,NULL, sizeof(int16_t), 0, 1000,   300},
		{ "acc",  	 NULL, offsetof(train_config_t, inertiacnf.acc),   	NULL,NULL, sizeof(int16_t), 0, 1000,   200},

		{"en_inertia", NULL, offsetof(train_config_t,enable_inertia),	NULL,NULL, sizeof(uint8_t), 0, 1, 0},
		{"en_pid",     NULL, offsetof(train_config_t,enable_pid),		NULL,NULL, sizeof(uint8_t), 0, 1, 0 /*1*/},
		{"notify_spd", NULL, offsetof(train_config_t,notify_speed),     NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"notify_pose",NULL, offsetof(train_config_t,notify_pose),      NULL,NULL, sizeof(uint8_t), 0, 1, 0},
		{"bemfIIR",    NULL, offsetof(train_config_t,bemfIIR),   	    NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"postIIR",    NULL, offsetof(train_config_t,postIIR ),   	    NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"fix_bemf",   NULL, offsetof(train_config_t,fix_bemf),   	    NULL,NULL, sizeof(uint8_t), 0, 1, 1},
		{"volt_policy",NULL, offsetof(train_config_t,volt_policy),      NULL,NULL, sizeof(uint8_t), 0, 1, 2 /*0*/},
		{"en_spd2pow", NULL, offsetof(train_config_t,en_spd2pow),       NULL,NULL, sizeof(uint8_t), 0, 1, 0 /*1*/},
		{"min_power",  NULL, offsetof(train_config_t,min_power),        NULL,NULL, sizeof(uint8_t), 0, 80, 40},

		{ NULL,     NULL,0,    NULL,NULL, 0, 0, 0,   0}
};

