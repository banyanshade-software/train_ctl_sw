//
//  ledio.c
//  train_throttle
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include "ledio.h"
#include "led.h"
#include "../misc.h"
//#include "trainctl_iface.h"
#include "../railconfig.h"

#ifndef BOARD_HAS_LED
#error BOARD_HAS_LED not defined, remove this file from build
#endif

void led_io(uint8_t lednum, uint8_t v)
{
#ifndef TRAIN_SIMU
	const led_config_t *cled = get_led_cnf(lednum);
	if (!cled) {
		itm_debug1(DBG_LED|DBG_ERR, "bad led", lednum);
		Error_Handler();
		//HardFault_Handler();
	}
	HAL_GPIO_WritePin(cled->port_led, cled->pin_led, (v) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
#endif
}

