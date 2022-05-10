/*
 * statval.h
 *
 *  Created on: Oct 28, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


#ifndef STATVAL_H_
#define STATVAL_H_

#include <sys/types.h>
#include "trainctl_config.h"

#define REDUCE_STAT 1

typedef struct statiteraror {
    int midx;
    int tidx;
    int vidx;
} stat_iterator_t;


int stat_iterator_reset(stat_iterator_t *it);
int stat_iterator_next(stat_iterator_t *it);

int32_t stat_val_get(stat_iterator_t *step, int *pdone);



int get_val_info(stat_iterator_t *step, off_t *poffset, int *plen, int *pidx,
    const char  **pzName);



typedef struct {
    void *ptr;
    off_t off;
    int8_t   l;
#ifdef  HOST_SIDE
    const char *param_name;
#endif
} stat_val_t;


#ifdef HOST_SIDE
#define _P(_n) ,_n
#else
#define _P(_n)
#endif

extern const stat_val_t statval_ctrl[];
extern const stat_val_t statval_spdctrl[];
extern const stat_val_t statval_canton[];
#ifdef BOARD_HAS_INA3221
extern const stat_val_t statval_ina3221[];
#endif
extern uint32_t gtick;

#endif /* STATVAL_H_ */
