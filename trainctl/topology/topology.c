/*
 * inertia.c
 *
 *  Created on: Jun 21, 2021
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2021
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include <stddef.h>
#include <memory.h>
#include "railconfig.h"
#include "topology.h"


int _blk_num_for_sub_num(int subnum)
{
	if (subnum == 2) return 0;
	if (subnum == 1) return 1;
	if (subnum == 0) return 2;
	return -1;
}


int _next_block_num(int blknum, uint8_t left)
{
	if ((0)) return -1; // XXX
	switch (blknum) {
	case 0:
		return left ? 	-1 : 1;
	case 1:
		return left ?	(topology_get_turnout(0) ? 2 : 0)  : -1;
	case 2:
		return left ?   -1 : 1;
	default:
		return -1;
	}
}

int get_blk_len(int blknum)
{
	switch (blknum) {
	case 0:
		return 70;
	case 1:
		return 40;
	case 2:
		return 50;
	default:
		return 10;
	}
}


// --------------------------------------------------------------------------------------

static volatile uint32_t turnoutvals = 0; // bit field

void topolgy_set_turnout(int tn, int v)
{
	if (tn >= NUM_TURNOUTS) return;
	if (tn<0) return;
	if (tn>31) return;

	if (v) {
		__sync_fetch_and_or(&turnoutvals, (1<<tn));
	} else {
		__sync_fetch_and_and(&turnoutvals, ~(1<<tn));
	}
	itm_debug3(DBG_TURNOUT, "tt",tn,v, topology_get_turnout(tn));
}
int topology_get_turnout(int tn)
{
	if (tn >= NUM_TURNOUTS) return 0;
	if (tn<0) return 0;
	if (tn>31) return 0;

	uint32_t b = turnoutvals;
	return (b & (1<<tn)) ? 1 : 0;
}


