/*
 * monitoring.c
 *
 *  Created on: Feb 25, 2023
 *      Author: danielbraun
 */

#include "main.h"
#include "cmsis_os.h"
#include "monitoring.h"

extern void itmTraceSend(int ch, uint8_t type,  uint8_t data)
{
	if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&      /* ITM enabled */
	    ((ITM->TER & (1<<ch)           ) != 0UL)   )  {   /* ITM Port #0 enabled */

		while (ITM->PORT[ch].u32 == 0UL)  {
			__NOP();
		}
		uint16_t v = (type<<8) | data;
		ITM->PORT[ch].u16 = v;
	}
}
