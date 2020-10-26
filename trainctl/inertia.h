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

#ifndef INERTIA_H_
#define INERTIA_H_


typedef struct {
	int16_t dec; // power% per second
	int16_t acc; // power% per second
} inertia_config_t;

typedef struct {
	int16_t target;
    int16_t cur;
} inertia_vars_t;

int16_t inertia_value(const inertia_config_t *cnf, inertia_vars_t *var, uint16_t elapsed_ticks, int *pchanged);
void    inertia_set_target(const inertia_config_t *cnf, inertia_vars_t *var, int16_t v);
void    inertia_reset(const inertia_config_t *cnf, inertia_vars_t *var);


#endif /* INERTIA_H_ */
