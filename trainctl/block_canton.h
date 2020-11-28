/*
 * block_canton.h
 *
 *  Created on: Nov 27, 2020
 *      Author: danielbraun
 */

#ifndef BLOCK_CANTON_H_
#define BLOCK_CANTON_H_


#include <stdint.h>

struct blk_side {
	uint8_t a;
	uint8_t b;
	uint8_t turnout;
};
typedef struct block_canton_config {
	struct blk_side left;
	struct blk_side right;
	uint8_t len;
} block_canton_config_t;


#define BLKCNT_STATUS_FREE 			0
#define BLKCNT_STATUS_NEIGHBOOR 	1
#define BLKCNT_STATUS_RELEASED	 	2
#define BLKCNT_STATUS_BUSY 			0xFF

typedef struct block_canton_vars {
	uint8_t trainidx;
	uint8_t status;
} block_canton_vars_t;


void block_canton_reset(const block_canton_config_t *c, block_canton_vars_t *v);


void block_canton_exit(uint8_t numtrain, uint8_t numcanton);
void block_canton_enter(uint8_t numtrain, uint8_t numcanton);
void block_canton_get_next(uint8_t numcanton, int8_t dir, uint8_t *pnext, int8_t *pnextdir);


#endif /* BLOCK_CANTON_H_ */
