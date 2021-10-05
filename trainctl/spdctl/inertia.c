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

#include "misc.h"
#include "inertia.h"

void inertia_reset(int tidx, _UNUSED_ const inertia_config_t *cnf, inertia_vars_t *vars)
{
	itm_debug1(DBG_INERTIA, "i/rst", tidx);
	vars->target = 0;
	vars->cur = 0;
}

int16_t inertia_value(int tidx, const inertia_config_t *config, inertia_vars_t *vars, int *pchanged)
{
	int st =  SIGNOF(vars->target);
	int sc =  SIGNOF(vars->cur);
	int inc;

	int32_t dt10 = (10*1000)/cur_freqhz;

    if (pchanged) *pchanged = 0;
    if (vars->target == vars->cur/10) {
    	itm_debug1(DBG_INERTIA, "i/no chg", tidx);
    	return vars->target;
    }

    itm_debug3(DBG_INERTIA, "i/val", tidx, vars->cur, vars->target);

	if (st*sc >= 0) {
		// same direction
		if (abs(vars->target*10)>abs(vars->cur)) {
			// acceleration
			inc = (config->acc * dt10) / 10000;
			inc = MIN(inc, abs(vars->target*10)-abs(vars->cur));
			inc = sc * inc;
	    	itm_debug3(DBG_INERTIA, "i/acc", tidx, inc, dt10);
		} else {
			// deceleration
			inc = (config->dec * dt10) / 10000;
			inc = MIN(inc, -abs(vars->target*10)+abs(vars->cur));
			inc = -sc * inc;
	    	itm_debug3(DBG_INERTIA, "i/dec", tidx, inc, dt10);
		}
	} else {
		// dir change
		inc = config->dec * dt10 / 10000;
        inc = MIN(inc, abs(vars->target*10-vars->cur));
        inc = -1 * sc * inc;
		itm_debug2(DBG_INERTIA, "i/dir chg", tidx, inc);
	}
	int vold = vars->cur/10;
	vars->cur += inc;
	int16_t vnew = vars->cur/10;
    if (pchanged) *pchanged = (vnew==vold) ? 0 : 1;
	return vnew;
}

