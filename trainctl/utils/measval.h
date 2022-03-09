/*
 * measval.h
 *
 *  Created on: Mar 6, 2022
 *      Author: danielbraun
 */

#ifndef UTILS_MEASVAL_H_
#define UTILS_MEASVAL_H_


typedef struct {
	int16_t count;
	int16_t res; // reserved / or private use
	int32_t sum;
	int32_t min;
	int32_t max;
	//int32_t sumsqr;
} measval_t;


static inline void measval_addvalue(measval_t *m, int32_t v)
{
	m->count++;
	m->sum += v;
	//m->sumsqr += v*v;
	if (1==m->count) {
		m->min = m->max = v;
	} else {
		if (v<m->min) m->min=v;
		if (v>m->max) m->max=v;
	}
}

static inline int32_t measval_avg(measval_t *m)
{
	if (!m->count) return 0;
	return m->sum / m->count;
}
#endif /* UTILS_MEASVAL_H_ */
