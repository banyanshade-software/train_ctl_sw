/*
 * inertia.h
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

#ifndef INERTIA_H_
#define INERTIA_H_


#include "../config/conf_locomotive.h"

/*
typedef struct {
	int16_t dec; // power% per second
	int16_t acc; // power% per second
} inertia_config_t;
*/

typedef struct {
	int16_t target;
    int16_t cur100;
} inertia_vars_t;

int16_t inertia_value(int tidx, const conf_locomotive_t *lconf, inertia_vars_t *var, int *pchanged);
void    inertia_reset(int tidx, inertia_vars_t *var);

static inline void inertia_set_target(_UNUSED_ int tidx, _UNUSED_ const struct conf_inertia *cnf, inertia_vars_t *vars, int16_t v)
{
	itm_debug2(DBG_INERTIA, "i-targ", tidx, v);
	vars->target = v;
}
static inline void inertia_temporary_deactivated(_UNUSED_ int tidx, _UNUSED_ const struct conf_inertia *cnf,  inertia_vars_t *vars, int16_t v)
{
	vars->cur100 = v * 100;
}

#endif /* INERTIA_H_ */
