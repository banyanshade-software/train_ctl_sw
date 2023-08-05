/*
 * canton_siggen.h
 *
 *  Created on: Aug 4, 2023
 *      Author: danielbraun
 */

#ifndef LOW_CANTON_SIGGEN_H_
#define LOW_CANTON_SIGGEN_H_


#include "canton.h"
#include "cantonP.h"
#include "../config/conf_canton.h"

void start_signal_gen(const conf_canton_t *cconf, canton_vars_t *cvars, uint16_t p);

#endif /* LOW_CANTON_SIGGEN_H_ */
