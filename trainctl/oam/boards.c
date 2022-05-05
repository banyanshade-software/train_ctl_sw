/*
 * boards.c
 *
 *  Created on: Apr 3, 2022
 *      Author: danielbraun
 */


#include <stdint.h>
#include "trainctl_config.h"

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
#ifndef BOARD_CAN_BE_MASTER
	return 0;
#endif
    return 1; // TODO there should be only one master
}

// XXX BOARD_NUMBER should be removed
static int _boardNumber = BOARD_NUMBER;

void oam_localBoardNum_set(uint8_t bnum)
{
	if (oam_isMaster()) {
		FatalError("mBDN", "master calling local_board_num", Error_BrdSlvMaster);
		return;
	}
    _boardNumber = bnum;
}


int oam_localBoardNum(void)
{
	if (oam_isMaster()) {
		return 0;
	}
    return _boardNumber;
}



