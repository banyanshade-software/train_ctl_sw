/*
 * param.c
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


#include <stddef.h>
#include <string.h>
#include "param.h"

static param_t *get_param_def(const param_t *rgpar, const char *n)
{
	for (param_t *p = (param_t*) rgpar; p->name; p++) {
		if (!strcmp(p->name, n)) return p;
	}
	return NULL;
}

int param_get_value(const param_t *params, void *ptr, const char *name, int32_t *pvalue, int32_t *pdef, int32_t *pmin, int32_t *pmax)
{
	param_t *prm = get_param_def(params, name);
	if (!prm) {
		return 10;
	}
	if (pmin) *pmin = prm->min;
	if (pmax) *pmax = prm->max;
	if (pdef) *pdef = prm->def;
	if (pvalue) {
		if (prm->getter) {
			*pvalue = prm->getter(prm);
		} else {
			uint8_t *p = ptr;
			if (prm->ptr) p = prm->ptr;
			p += prm->offset;
			if (!p) return 11;
			if ((int)p<0x100) return 12; // probably an error if low ptr

			int8_t v8; int16_t v16; int32_t v32;
			switch (prm->size) {
			case 1:
				v8 = *((int8_t *)p);
				v32 = (int32_t)v8;
				break;
			case 2:
				v16 = *((int16_t *)p);
				v32 = (int32_t)v16;
				break;
			case 4:
				v32 = *((int32_t *)p);
				break;
			default:
				return 12;
			}
			*pvalue = v32;
		}
	}
	return 0;
}
int param_set_value(const param_t *params, void *ptr, const char *name, int32_t value)
{
	param_t *prm = get_param_def(params, name);
	if (!prm) {
		return 10;
	}
	if (prm->setter) {
		//if ((1)) return 42;
		prm->setter(prm, value);
	} else {
		uint8_t *p = ptr;
		if (prm->ptr) p = prm->ptr;
		p += prm->offset;
		if (!p) return 11;
		if ((int)p<0x100) return 12; // probably an error if low ptr

		int8_t v8; int16_t v16; int32_t v32;
		v32 = value;
		switch (prm->size) {
		case 1:
			v8 = (int8_t) v32;
			*((int8_t *)p) = v8;
			break;
		case 2:
			v16 = (int16_t) v32;
			*((int16_t *)p) = v16;
			break;
		case 4:
			*((int32_t *)p) = v32;
			break;
		default:
			return 12;
		}
	}
	return 0;
}

