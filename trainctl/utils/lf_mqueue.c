/*
 * lf_mqueue.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


#include <stdint.h>
#include <memory.h>
#include "lf_mqueue.h"
//#include <iot_atomic.h>

// https://electronics.stackexchange.com/questions/13100/replacement-for-queues-in-rtos#13102

void mqf_clear(mqf_t *m)
{
	m->head = 0;
	m->tail = 0;
}

int mqf_len (mqf_t *m)
{
	if (m->head >= m->tail) {
		return (m->head - m->tail);
	} else   {
		return m->num - ((m->num - m->tail) + m->head);
	}
}


int mqf_write(mqf_t *m, void *ptr)
{
	if (m->num == mqf_len(m)) return -1;
	void *p = &(m->msgbuf[m->head*m->msgsiz]);
	memcpy(p, ptr, m->msgsiz);
	//__barrier();
	__sync_fetch_and_add(&(m->head), 1);
	//AtomicInc(p->head);
	return 0;
}

int mqf_read(mqf_t *m, void *ptr)
{
	if (!mqf_len(m)) return -1;
	void *p = &(m->msgbuf[m->tail*m->msgsiz]);
	memcpy(ptr, p, m->msgsiz);
	//__barrier();
	__sync_fetch_and_add(&(m->tail), 1);
	return 0;
}
