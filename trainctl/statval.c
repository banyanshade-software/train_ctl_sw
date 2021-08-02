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
    const stat_val_t *statval;
    int numelem;
    size_t structsize;
    int numtrain; // or canton
} meta_stat_t;

#define NUM_META 3
static meta_stat_t meta[NUM_META+1] = {
    {statval_ctrl, 0, 0, NUM_TRAINS},
    {statval_spdctrl, 0, 0, NUM_TRAINS},
    {statval_canton, 0, 0, NUM_LOCAL_CANTONS_HW},
    //{statval_ina3221, 0, 0, 12},
    {NULL, 0, 0, 0}
};

static void build_meta(void)
{
    meta_stat_t *m;
    for (m = &meta[0]; m->statval; m++) {
        const stat_val_t *v;
        int n=0;
        for (v = &(m->statval[0]) ; v->ptr; v++) {
            n++;
        }
        m->numelem = n;
        m->structsize = v->off;
    }
}


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


int stat_iterator_reset(stat_iterator_t *it)
{
    it->midx = 0;
    it->tidx = 0;
    it->vidx = 0;
    if (!NUM_META) return -1;
    return 0;
}

int  stat_iterator_next(stat_iterator_t *it)
{
    meta_stat_t *m = &meta[it->midx];
    it->vidx ++;
    if (it->vidx >= m->numelem) {
        it->vidx = 0;
        it->tidx ++;
    }
    if (it->tidx >= m->numtrain) {
        it->tidx = 0;
        it->midx ++;
    }
    if (it->midx >= NUM_META) return -1;
    return 0;
}

int32_t stat_val_get(stat_iterator_t *step, int *pdone)
{
	*pdone = 0;
    meta_stat_t *m = &meta[step->midx];
    const stat_val_t *sv = &(m->statval[step->vidx]);
    uint8_t *cptr = (uint8_t *)(sv->ptr);
    uint8_t *vars = cptr + step->tidx * m->structsize;
    return _getval(vars, sv->off, sv->l);
	return 0;
}
    
    
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
}
#endif


int get_val_info(int step, off_t *poffset, int *plen, int *ptridx, int *pcntidx, const char  **pzName, int numtrain, int numcanton)
{
    static int first = 1;
    if (first) {
        build_meta();
        first=0;
    }
#if 0
    *ptridx = -1;
    *pcntidx = -1;
    *pzName = NULL;
    
    if (!numvaltrain) return 0;
    
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
#endif
    return 0;
}



