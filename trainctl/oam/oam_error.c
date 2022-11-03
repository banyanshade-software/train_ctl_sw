/*
 * oam_error.c
 *
 *  Created on: Apr 24, 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_ERROR_C_
#define OAM_OAM_ERROR_C_

#include <stdint.h>
#include "misc.h"
#include "oam_error.h"
#include "../low/turnout.h"

const char *_fatal = NULL;

__weak void local_ui_fatal(void)
{

}

#ifndef TRAIN_SIMU
void FatalError( const char *short4lettersmsg, _UNUSED_ const char *longmsg, _UNUSED_ enum fatal_error_code errcode)
{
	if (!_fatal) {
		_fatal = short4lettersmsg;
#ifdef BOARD_HAS_TURNOUTS
		TurnoutEmergencyStop();
#endif
		local_ui_fatal();
		__disable_irq();
		for (;;) {
			// stop
		}
	}
}
#endif

// Error_Handler

#endif /* OAM_OAM_ERROR_C_ */
