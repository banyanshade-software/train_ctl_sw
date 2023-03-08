/*
 * monitoring.c
 *
 *  Created on: Feb 25, 2023
 *      Author: danielbraun
 */

#include "main.h"
#include "cmsis_os.h"
#include "monitoring.h"

#ifdef ENABLE_TRACES

extern void itmTraceSend(int ch, uint8_t type,  uint8_t data)
{
	if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&      /* ITM enabled */
	    ((ITM->TER & (1<<ch)           ) != 0UL)   )  {   /* ITM Port #0 enabled */

		int n=0;
		while (ITM->PORT[ch].u32 == 0UL)  {
			if (n++>TRACE_MAW_WAIT) return;
			__NOP();
		}
		uint16_t v = (type<<8) | data;
		ITM->PORT[ch].u16 = v;
	}
}

void itmTraceTaskCreate(int tcbnnum, int tasknum, char *taskname)
{
	(void)tcbnnum; (void) tasknum; (void)taskname;
}

#endif
