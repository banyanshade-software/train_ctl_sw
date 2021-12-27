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


const stat_val_t statval_glob[] = {
    { &gtick, 0,           sizeof(uint32_t)   _P("tick")},
    { NULL,  0, 0 _P(NULL)}
};

#define NUM_META 4
static meta_stat_t meta[NUM_META+1] = {
    {statval_glob, 0, 0, 1},
    // stats only on first 3 trains and first 4 blocks
#ifdef BOARD_HAS_CTRL
    {statval_ctrl, 0, 0, MIN(3,NUM_TRAINS)},
    {statval_spdctrl, 0, 0, MIN(3,NUM_TRAINS)},
#endif
#ifdef BOARD_HAS_CANTON
    {statval_canton, 0, 0, MIN(4,NUM_LOCAL_CANTONS_HW)},
#endif
    //{statval_ina3221, 0, 0, 12},
    {NULL, 0, 0, 0}
};

static volatile int meta_ok = 0;
static void build_meta(void)
{
    int ok = __sync_fetch_and_or(&meta_ok, 1);
    if (ok) return;
    
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
	default:
		v32=0;
		break;
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
    static int first = 1;
    if (first) {
        build_meta();
        first=0;
    }
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

#ifdef HOST_SIDE
int get_val_info(stat_iterator_t *step,
                 off_t *poffset, int *plen, int *pidx, const char  **pzName)
{
    static int first = 1;
    if (first) {
        build_meta();
        first=0;
    }
    meta_stat_t *m = &meta[step->midx];
    const stat_val_t *sv = &(m->statval[step->vidx]);
    //uint8_t *cptr = (uint8_t *)(sv->ptr);
    //uint8_t *vars = cptr + step->tidx * m->structsize;
    
    *poffset = sv->off;
    *plen = sv->l;
    *pidx = step->tidx;
    *pzName = sv->param_name;

    return 0;
}

#endif //HOST_SIDE

