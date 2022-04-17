/*
 * inertia.c
 *
 *  Created on: Oct 2, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


/*
 * inertia.h : control of inertia simulation
 * 			model train are very light and thus don't have inertia (they brake/accelerate) nearly immediatly
 * 			thus we add inertia simulation, with a max acceleration and deceleration (braking)
 */


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../misc.h"
#include "inertia.h"



#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif


void inertia_reset(int tidx, _UNUSED_ const struct conf_inertia *cnf, inertia_vars_t *vars)
{
	itm_debug1(DBG_INERTIA, "i/rst", tidx);
	vars->target = 0;
	vars->cur100 = 0;
}

int16_t inertia_value(int tidx, const struct conf_inertia *config, inertia_vars_t *vars, int *pchanged)
{
	int st =  SIGNOF0(vars->target);
	int sc =  SIGNOF0(vars->cur100);
	int inc;

	int32_t dt100 = (100*1000)/tsktick_freqhz;
    int32_t trga100 = abs(vars->target*100);
    
    if (pchanged) *pchanged = 0;
    if (vars->target == vars->cur100/100) {
    	itm_debug1(DBG_INERTIA, "i/no chg", tidx);
    	return vars->target;
    }

    itm_debug3(DBG_INERTIA, "i/val", tidx, vars->cur100, vars->target);

	if (st*sc >= 0) {
		// same direction
        if (!sc) sc = st; // case where cur=0
		if (trga100 > abs(vars->cur100)) {
			// acceleration
			inc = (config->acc * dt100) / 10000;
			inc = MIN(inc, trga100-abs(vars->cur100));
			inc = sc * inc;
	    	itm_debug3(DBG_INERTIA, "i/acc", tidx, inc, dt100);
		} else {
			// deceleration
			inc = (config->dec * dt100) / 10000;
			inc = MIN(inc, -trga100+abs(vars->cur100));
			inc = -sc * inc;
	    	itm_debug3(DBG_INERTIA, "i/dec", tidx, inc, dt100);
		}
	} else {
		// dir change
		inc = config->dec * dt100 / 10000;
        inc = MIN(inc, abs(vars->target*100-vars->cur100));
        inc = -1 * sc * inc;
		itm_debug2(DBG_INERTIA, "i/dir chg", tidx, inc);
	}
	int vold = vars->cur100/100;
	vars->cur100 += inc;
	int16_t vnew = vars->cur100/100;
    if (pchanged) *pchanged = (vnew==vold) ? 0 : 1;
	return vnew;
}

