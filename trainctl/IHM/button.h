/*
 * button.h
 *
 *  Created on: Jun 19, 2022
 *      Author: danielbraun
 */

#ifndef IHM_BUTTON_H_
#define IHM_BUTTON_H_

typedef struct {
	GPIO_TypeDef *port;
	uint16_t     pin;
} ihm_button_def_t;

typedef struct {
	const ihm_button_def_t *def;
	uint8_t laststate;
	uint32_t tpush;
} ihm_button_t;

uint8_t ihm_poll_button(ihm_button_t *b, uint32_t tick);

#endif /* IHM_BUTTON_H_ */
