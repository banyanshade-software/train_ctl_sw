/*
 * block_canton.c
 *
 *  Created on: Nov 27, 2020
 *      Author: danielbraun
 */

#if 0
#include <memory.h>
#include "block_canton.h"
#include "railconfig.h"

void block_canton_reset(const block_canton_config_t *c,  block_canton_vars_t *v)
{
	memset(v, 0, sizeof(*v));
	v->status = BLKCNT_STATUS_FREE;
	v->trainidx = 0xFF;
}


static void blkcanton_problem(void)
{

}

void block_canton_exit(uint8_t numtrain, uint8_t numcanton)
{
	USE_BLOCK_CANTON(numcanton);
    (void) bcconf; //unused
	if (bcvars->trainidx != numtrain) blkcanton_problem();
	bcvars->trainidx = 0xFF;
	bcvars->status = BLKCNT_STATUS_RELEASED;
}
void block_canton_enter(uint8_t numtrain, uint8_t numcanton)
{
	USE_BLOCK_CANTON(numcanton);
    (void) bcconf; //unused
	if (bcvars->trainidx != 0xFF) blkcanton_problem();
	bcvars->trainidx = numtrain;
	bcvars->status = BLKCNT_STATUS_BUSY;
}

void block_canton_get_next(uint8_t numcanton, int8_t dir, uint8_t *pnext, int8_t *pnextdir)
{
	const struct blk_side *side;
	int n = 0xFF;
	USE_BLOCK_CANTON(numcanton);
    (void) bcvars; //unused
	if (dir>0) {
		side = &bcconf->right;
	} else {
		side = &bcconf->left;
	}
	if ((side->a != 0xFF) && (side->b != 0xFF)) {
		if (side->turnout == 0xFF) {
			blkcanton_problem();
			n = side->a;
		} else {
			USE_TURNOUT(side->turnout);
            (void) aconf; // unused
			switch (avars->value) {
			case 1: n = side->b; break;
			case -1: n = side->a; break;
			default: blkcanton_problem(); n = side->a;
			}
		}
	} else if (side->a) {
		n = side->a;
	} else if (side->b) {
		n = side->b;
	} else {
		*pnext = 0xFF;
		*pnextdir = 0;
		return;
	}
	*pnext = n;
    if (n == 0xFF) {
        *pnextdir = 0;
        return;
    }
	const block_canton_config_t *nconf = get_block_canton_cnf(n);
	if (nconf->left.a == numcanton || nconf->left.b == numcanton) {
		*pnextdir = 1;
	} else if (nconf->right.a == numcanton || nconf->right.b == numcanton) {
		*pnextdir = -1;
	} else {
		blkcanton_problem();
		*pnextdir = 1;
	}

}
#endif
