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
//#include "train.h"
#include "spdctl/spdctl.h"
#include "low/canton_config.h"
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
		/*{ offsetof(train_vars_t, pidvars.last_err), 4   _P("pid_last_err")},
		{ offsetof(train_vars_t, pidvars.sume), 4       _P("pid_sum_e")},
		{ offsetof(train_vars_t, pidvars.target_v), 4   _P("pid_target")},
		{ offsetof(train_vars_t, inertiavars.target), 2 _P("ine_t")},
		{ offsetof(train_vars_t, inertiavars.cur), 2    _P("ine_c")},
		{ offsetof(train_vars_t, target_speed), 2       _P("target_speed")},
		{ offsetof(train_vars_t, last_speed), 2         _P("curspeed")},
        { offsetof(train_vars_t, position_estimate), 4  _P("train0_pose")},
        { offsetof(train_vars_t, bemfiir), 4            _P("bemfiir_centivolts")},
*/};

static const stat_val_t statvalcanton[] = {
/*		{ offsetof(canton_vars_t, cur_dir) , 1          _P("dir")},
        { offsetof(canton_vars_t, cur_voltidx) , 1      _P("vidx")},
		{ offsetof(canton_vars_t, cur_pwm_duty) , 2     _P("canton_%d_pwm")},
		{ offsetof(canton_vars_t, bemf_centivolt) , 4   _P("canton_%d_bemfcentivolt")},
        { offsetof(canton_vars_t, selected_centivolt),2 _P("canton_%d_centivolts")},
        { offsetof(canton_vars_t, von_centivolt) , 2    _P("canton_%d_centivon")},
        { offsetof(canton_vars_t, i_on) , 2   		    _P("canton_%d_ion")},
        { offsetof(canton_vars_t, i_off) , 2            _P("canton_%d_ioff")},
*/};

_UNUSED_ static int32_t _getval(void *ptr, off_t offset, int l)
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

static const int numvaltrain = sizeof(statvaltrain)/sizeof(statvaltrain[0]);
static const int numvalcanton = sizeof(statvalcanton)/sizeof(statvalcanton[0]);



int32_t stat_val_get(int step, int *pdone)
{
	*pdone = 1;
	return 0;
#if 0
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
#endif
}

int get_val_info(int step, off_t *poffset, int *plen, int *ptridx, int *pcntidx, const char  **pzName, int numtrain, int numcanton)
{
    *ptridx = -1;
    *pcntidx = -1;
    *pzName = NULL;
    
    int nt = step / numvaltrain;
    if (nt>=NUM_TRAINS) {
        // canton
        step -= NUM_TRAINS*numvaltrain;
        int nc = step / numvalcanton;
        if (nc >= NUM_LOCAL_CANTONS_SW) {
            // done
            return -1;
        }
        int idx = step % numvalcanton;
        const stat_val_t *sv = &statvalcanton[idx];
        *poffset = sv->off;
        *plen = sv->l;
        *pcntidx = nc;
#ifdef HOST_SIDE
        *pzName = sv->param_name;
#endif
    } else {
        int idx = step % numvaltrain;
        const stat_val_t *sv = &statvaltrain[idx];
        *poffset = sv->off;
        *plen = sv->l;
        *ptridx = nt;
#ifdef HOST_SIDE
        *pzName = sv->param_name;
#endif
    }
    return 0;
}
