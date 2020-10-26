/*
 * txrxcmd.h
 *
 *  Created on: Oct 25, 2020
 *      Author: danielbraun
 */

#ifndef TXRXCMD_H_
#define TXRXCMD_H_

#include "trainctl_iface.h"

void txrx_process_char(uint8_t c, uint8_t *respbuf, int *replen);

void frame_send_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen);


static inline void txframe_send_response(frame_msg_t *m, int len)
{
	m->t = TXFRAME_TYPE_RESP;
	if (len) m->len = len;
	txframe_send(m, 0);
}


static inline void txframe_send_notif(frame_msg_t *m, int len)
{
	m->t = TXFRAME_TYPE_NOTIF;
	if (len) m->len = len;
	txframe_send(m, 1);
}


static inline void txframe_send_debug(frame_msg_t *m, int len)
{
	m->t = TXFRAME_TYPE_DEBUG;
	if (len) m->len = len;
	txframe_send(m, 2);
}

#endif /* TXRXCMD_H_ */
