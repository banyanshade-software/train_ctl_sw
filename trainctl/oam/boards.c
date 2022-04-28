/*
 * boards.c
 *
 *  Created on: Apr 3, 2022
 *      Author: danielbraun
 */



#include <stdint.h>
#include <memory.h>

#include "../misc.h"
#include "../msg/trainmsg.h"

#include "boards.h"

// 0x3438510f

#if 0
int boardIdToBoardNum(uint32_t uuid)
{
	// TODO we hardcode it for now
	if ((0)) {
	} else if (0x33a640b == uuid) {
		return 0;
	} else if (0x12d6661 == uuid) {
		return 1;
    } else if (42 == uuid) {
        // simu
        return 0;
    } else {
		FatalError("BRD?", "unknown brd id", Error_BrdUnknown);
	}
	return -1;
}
#endif



int oam_isMaster(void)
{
    return 1; // TODO
}

// XXX BOARD_NUMBER should be removed
static int _boardNumber = BOARD_NUMBER;

void oam_store_slave_local_boardnum(uint8_t bnum)
{
    _boardNumber = bnum;
}


int oam_localBoardNum(void)
{
    return _boardNumber;

}
