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

int boardIdToBoardNum(uint32_t uuid)
{
	// TODO we hardcode it for now
	if ((0)) {
	} else if (0x33a640b == uuid) {
		return 0;
	} else if (0x12d6661 == uuid) {
		return 1;
	} else {
		Error_Handler();
	}
	return -1;
}
