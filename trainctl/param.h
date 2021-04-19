/*
 * param.h
 *
 *  Created on: Sep 21, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/*
 * param.h  : handling of user modifiable parameters
 *            allows acces to any field in a struct or global variables - or function setters/getter
 */

#ifndef SRC_PARAM_H_
#define SRC_PARAM_H_

#include <sys/types.h>
#include <stddef.h>
#include "trainctl_iface.h"

typedef struct param {
	const char *name;
	void *ptr;
	off_t offset;
	int32_t (*getter)(struct param *);
	void (*setter)(struct param *, int32_t);
	int size;
	int min;
	int max;
	int def;
} param_t;

int param_get_value(const param_t *params, void *ptr, const char *name, int32_t *pvalue, int32_t *pdef, int32_t *pmin, int32_t *pmax);
int param_set_value(const param_t *params, void *ptr, const char *name, int32_t value);

extern const param_t train_params[];

#endif /* SRC_PARAM_H_ */
