//
//  ledio.c
//  train_throttle
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include "ledio.h"
#include "led.h"
#include "misc.h"
#include "trainctl_iface.h"
#include "railconfig.h"

void led_io(uint8_t lednum, uint8_t v)
{
	const led_config_t *cled = get_led_cnf(lednum);
	if (!cled) {
		itm_debug1(DBG_LED|DBG_ERR, "bad led", lednum);
	}
	HAL_GPIO_WritePin(cled->port_led, cled->pin_led, (v) ? GPIO_PIN_SET : GPIO_PIN_RESET);

}

