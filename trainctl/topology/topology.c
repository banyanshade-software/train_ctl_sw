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
	if (blknum == 0) {
		return 1;
	}
	return 0xFF;
}
