/*
 * button.c
 *
 *  Created on: Jun 19, 2022
 *      Author: danielbraun
 */

#include <stdint.h>
#include "../misc.h"
#include "button.h"


uint8_t ihm_poll_button(ihm_button_t *b, uint32_t tick)
{
	int v = HAL_GPIO_ReadPin(b->def->port, b->def->pin);
	v = !v; // button state : pressed = 1 (gpio = gnd)
	if (v && ! b->laststate) {
		b->tpush = tick;
		b->laststate = 1;
	} else if (b->laststate && !v) {
		b->laststate = 0;
		uint32_t dt = tick - b->tpush;
		if (dt<15) return 0;
		if (dt>1000) return 0;
		return 1;
	}
	return 0;
}
