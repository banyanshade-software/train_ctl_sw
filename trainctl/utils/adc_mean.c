//
//  adc_mean.c
//  train_throttle
//
//  Created by Daniel BRAUN on 14/02/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#include <stdint.h>
#include <memory.h>
#include "adc_mean.h"


void adc_mean_init(adc_mean_ctx_t *m)
{
    memset(m, 0, sizeof(*m));
}

#ifdef AVG_MEAN_AVERAGE
void adc_mean_add_value(adc_mean_ctx_t *m, uint16_t va, uint16_t vb)
{
	int16_t v = vb-va;
    m->n++;
    m->sum += v;
}

int16_t adc_mean_get_mean(adc_mean_ctx_t *m)
{
    if (!m->n) return 0;
    return (uint16_t) (m->sum / m->n);
}

#endif // AVG_MEAN_AVERAGE


#ifdef AVG_MEAN_CUSTOM
void adc_mean_add_value(adc_mean_ctx_t *m, uint16_t va, uint16_t vb)
{
	int16_t v = vb - va;
    if (v >= m->val) {
        m->val = (m->val + 7*v)/8;
    } else {
        uint16_t vn = m->val-v;
        if (vn > m->val/16) vn = m->val/16;
        m->val = m->val-vn;
    }
}
int16_t adc_mean_get_mean(adc_mean_ctx_t *m)
{
    return m->val;
}

#endif


#ifdef    AVG_MEAN_MAX
void adc_mean_add_value(adc_mean_ctx_t *m, uint16_t va, uint16_t vb)
{
	if (v > m->max) m->max = v;
}
uint16_t adc_mean_get_mean(adc_mean_ctx_t *m)
{
	return m->max;
}
#endif
