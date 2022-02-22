//
//  adc_mean.h
//  train_throttle
//
//  Created by Daniel BRAUN on 14/02/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef adc_mean_h
#define adc_mean_h

#include <stdint.h>

//#define AVG_MEAN_AVERAGE
#define AVG_MEAN_CUSTOM

typedef struct {
#ifdef AVG_MEAN_AVERAGE
    int n;
    uint32_t sum;
#endif
#ifdef AVG_MEAN_CUSTOM
    uint16_t val;
#endif
} adc_mean_ctx_t;

void adc_mean_init(adc_mean_ctx_t *);
void adc_mean_add_value(adc_mean_ctx_t *, uint16_t);
uint16_t adc_mean_get_mean(adc_mean_ctx_t *);

#endif /* adc_mean_h */
