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

int32_t stat_val_get(int step, int *pdone);



int get_val_info(int step, off_t *poffset, int *plen, int *ptridx, int *pcntidx, const char  **pzName, int numtrain, int numcanton);

#endif /* STATVAL_H_ */
