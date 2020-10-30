/*
 * statval.c
 *
 *  Created on: Oct 28, 2020
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

#include "statval.h"
#include "train.h"
#include "canton.h"
#include "railconfig.h"

typedef struct {
	off_t off;
	int8_t   l;
#ifdef  HOST_SIDE
    const char *param_name;
#endif
} stat_val_t;

#ifdef HOST_SIDE
#define _P(_n) ,_n
#else
#define _P(_n)
#endif

static const stat_val_t statvaltrain[] = {
		{ offsetof(train_vars_t, pidvars.last_err), 4   _P(NULL)},
		{ offsetof(train_vars_t, pidvars.sume), 4       _P(NULL)},
		{ offsetof(train_vars_t, pidvars.target_v), 4   _P(NULL)},
		{ offsetof(train_vars_t, inertiavars.target), 2 _P(NULL)},
		{ offsetof(train_vars_t, inertiavars.cur), 2    _P(NULL)},
		{ offsetof(train_vars_t, target_speed), 2       _P(NULL)},
		{ offsetof(train_vars_t, last_speed), 2         _P("curspeed")},
		{ offsetof(train_vars_t, position_estimate), 4  _P("train0_pose")},
};

static const stat_val_t statvalcanton[] = {
		{ offsetof(canton_vars_t, cur_dir) , 1          _P(NULL)},
        { offsetof(canton_vars_t, cur_voltidx) , 1      _P(NULL)},
		{ offsetof(canton_vars_t, cur_pwm_duty) , 2     _P("canton_%d_pwm")},
		{ offsetof(canton_vars_t, bemf_centivolt) , 4   _P("canton_%d_bemf")},
    { offsetof(canton_vars_t, selected_centivolt) , 4   _P("canton_%d_volts")},
    { offsetof(canton_vars_t, von_centivolt) , 4   _P("canton_%d_von")},

};

static int32_t _getval(void *ptr, off_t offset, int l)
{
	uint8_t *p8 = (uint8_t*)ptr;
	int32_t v32=0;
    int16_t v16; int8_t v8;
	switch(l) {
	case 1:
		v8 = p8[offset];
		v32 = (int32_t) v8;
		break;
	case 2:
		memcpy(&v16, p8+offset, 2);
		v32 = (int32_t) v16;
		break;
	case 4:
		memcpy(&v32, p8+offset, 4);
		break;
	}
	return v32;
}
int32_t stat_val_get(int step, int *pdone)
{
	static const int numvaltrain = sizeof(statvaltrain)/sizeof(statvaltrain[0]);
	static const int numvalcanton = sizeof(statvalcanton)/sizeof(statvalcanton[0]);
	*pdone = 0;
	int nt = step / numvaltrain;
	if (nt>=NUM_TRAINS) {
		// canton
		step -= NUM_TRAINS*numvaltrain;
		int nc = step / numvalcanton;
		if (nc >= NUM_CANTONS) {
			// done
			*pdone = 1;
			return 0;
		}
		int idx = step % numvalcanton;
		canton_vars_t *vars = get_canton_vars(nc);
		const stat_val_t *sv = &statvalcanton[idx];
		return _getval(vars, sv->off, sv->l);
	} else {
		int idx = step % numvaltrain;
		train_vars_t *vars = get_train_vars(nt);
		const stat_val_t *sv = &statvaltrain[idx];
		return _getval(vars, sv->off, sv->l);
	}
}
