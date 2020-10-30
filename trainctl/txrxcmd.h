/*
 * txrxcmd.h
 *
 *  Created on: Oct 25, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


/* txrxcmd.h : process command and send notification to control software
 * 		for now, control runs on mac, communicating with USB.
 * 		it is expected that the same command processing can be used
 * 		- on BT between main board and tablet / computer
 * 		- on I2C (or other) between main board and slave boards
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


static inline void txframe_send_stat(void)
{
    frame_msg_t m;
    m.t = TXFRAME_TYPE_STAT;
    txframe_send(&m, 1);
}

static inline void txframe_send_debug(frame_msg_t *m, int len)
{
	m->t = TXFRAME_TYPE_DEBUG;
	if (len) m->len = len;
	txframe_send(m, 2);
}


// buf should be long enough to store a int32_t with escape, so 8 bytes
int frame_gather_stat(int step, uint8_t *buf);
/*
 * send stat frame using callback function
 * frame header and end delimiter are NOT sent by frame_send_stat
 */
void frame_send_stat(void(*cb)(uint8_t *d, int l), uint32_t tick);

#endif /* TXRXCMD_H_ */
