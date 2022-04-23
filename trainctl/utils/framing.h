/*
 * framing.h
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */

#ifndef UTILS_FRAMING_H_
#define UTILS_FRAMING_H_

int txrx_frm_escape2(uint8_t *buf,  uint8_t *org, int len, int maxlen);


void frame_send_oscillo(_UNUSED_ void(*cb)(const uint8_t *d, int l));
void frame_send_stat(void(*cb)(const uint8_t *d, int l), uint32_t tick);



#endif /* UTILS_FRAMING_H_ */
