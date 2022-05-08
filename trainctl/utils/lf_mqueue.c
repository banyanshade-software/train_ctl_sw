/*
 * lf_mqueue.c
 *
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */


#include <stdint.h>
#include <memory.h>
#include "lf_mqueue.h"
#include "itm_debug.h"
#include "../oam/oam_error.h"

#define _UNUSED_ __attribute__((unused))

//#include <iot_atomic.h>

// https://electronics.stackexchange.com/questions/13100/replacement-for-queues-in-rtos#13102

void mqf_clear(mqf_t *m)
{
	m->head = 0;
	m->tail = 0;
	m->maxuse = 0;
}


static inline int _mqf_len(const mqf_t *m)
{
	int l;
	if (m->head >= m->tail) {
		l = (m->head - m->tail);
	} else   {
		l = m->num + m->head - m->tail;
	}
	if (l<0) {
		itm_debug1(DBG_ERR|DBG_MSG, "big pb", 1);
		FatalError("Qpb", "msgq big problem", Error_MsgQBig);
	}
	return l;
}
int mqf_len(const mqf_t *m)
{
	return _mqf_len(m);
}


void dump_msg(mqf_t *mq, int n);

void mqf_qfull(mqf_t *mq, _UNUSED_ int t)
{
	itm_debug1(DBG_ERR|DBG_MSG, "w/full", 0);
	for (;;) {
		static uint8_t dmp = 0;
		if (dmp) {
			dmp = 0;
			for (int i=0; i<_mqf_len(mq); i++) {
				dump_msg(mq, i);
			}
		}
	}
}
int mqf_write(mqf_t *m, void *ptr)
{
	int l = _mqf_len(m);
	if (l<0) FatalError("Ql", "msgq len problem", Error_MsgQLen);
	if (l > m->maxuse) m->maxuse = (int8_t) l;

    if (m->num == l) {
		itm_debug1(DBG_ERR|DBG_MSG, "w/full", 0);
		mqf_qfull(m,0);
        return -1;
    }
    if (m->num-1 == l) {
    	itm_debug1(DBG_MSG, "w/full1", m->silentdrop);
    	if (!m->silentdrop) mqf_qfull(m,1);
        return -1;
    }
	void *p = &(m->msgbuf[m->head*m->msgsiz]);
	memcpy(p, ptr, m->msgsiz);
	//__barrier();
    if (m->head == m->num-1) {
        m->head = 0;
    } else {
        __sync_fetch_and_add(&(m->head), 1);
    }
	//AtomicInc(p->head);
	return 0;
}

int mqf_read(mqf_t *m, void *ptr)
{
	if (!m) return -1;
	if (!_mqf_len(m)) return -1;
    void *p = &(m->msgbuf[m->tail*m->msgsiz]);
    memcpy(ptr, p, m->msgsiz);
    //__barrier();
    if (m->tail == m->num-1) {
        m->tail = 0;
    } else {
        __sync_fetch_and_add(&(m->tail), 1);
    }
    return 0;
}
