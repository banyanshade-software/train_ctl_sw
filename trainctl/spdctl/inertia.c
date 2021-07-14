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

void    inertia_reset(_UNUSED_ const inertia_config_t *cnf, inertia_vars_t *vars)
{
	vars->target = 0;
	vars->cur = 0;

}
void    inertia_set_target(_UNUSED_ const inertia_config_t *cnf, inertia_vars_t *vars, int16_t v)
{
	vars->target = v;
}

int16_t inertia_value(const inertia_config_t *config, inertia_vars_t *vars, uint16_t elapsed_ticks, int *pchanged)
{
	int st =  SIGNOF(vars->target);
	int sc =  SIGNOF(vars->cur);
	int inc;

    if (pchanged) *pchanged = 0;
    if (vars->target == vars->cur/10) return vars->target;
    //debug_info(0, "INER", vars->target, vars->cur);


	if (st*sc >= 0) {
		// same direction
		if (abs(vars->target*10)>abs(vars->cur)) {
			// acceleration
			inc = config->acc * elapsed_ticks / 1000;
			inc = MIN(inc, abs(vars->target*10)-abs(vars->cur));
			inc = sc * inc;
		} else {
			// deceleration
			inc = config->dec * elapsed_ticks / 1000;
			inc = MIN(inc, -abs(vars->target*10)+abs(vars->cur));
			inc = -sc * inc;
		}
	} else {
		// dir change
		inc = config->dec * elapsed_ticks / 1000;
        inc = MIN(inc, abs(vars->target*10-vars->cur));
        inc = -1 * sc * inc;
	}
	int vold = vars->cur/10;
	vars->cur += inc;
	int16_t vnew = vars->cur/10;
    if (pchanged) *pchanged = (vnew==vold) ? 0 : 1;
    //debug_info(0, "INC/c", inc, vars->cur);
	return vnew;
}

