/*
 * lf_mqueue.h
 *
 * lock-free (single producer, single consumer) fifo
 *  Created on: Apr 17, 2021
 *      Author: danielbraun
 */

#ifndef UTILS_LF_MQUEUE_H_
#define UTILS_LF_MQUEUE_H_

#include <stdint.h>


typedef struct {
	volatile uint16_t head;
	volatile uint16_t tail;
	uint8_t msgsiz;
	uint8_t num;
	uint8_t maxuse;	// TODO remove, max used len
	uint8_t silentdrop; // drop silently if full
	uint8_t *msgbuf;
} mqf_t;


// put msg queues in CCM ram, if any
//#define MQF_ATTRIB
#ifndef TRAIN_SIMU
#define MQF_ATTRIB __attribute__((section(".ccmram")))
#else
#define MQF_ATTRIB
#endif

#ifndef RECORD_MSG
#define RECORD_MSG 0
#endif

extern void record_msg_read(void *ptr);
extern void record_msg_write(void *ptr);


#define LFMQUEUE_DEF_H(_name, _type) 						\
    extern mqf_t  _name; 									\
    														\
	static inline int mqf_read_ ## _name(_type *ptr)		\
	{														\
		int rc = mqf_read(&_name, ptr);						\
    	if (RECORD_MSG && !rc) record_msg_read(ptr);		\
    	return rc;											\
	}														\
	static inline int mqf_write_ ## _name(_type *ptr)		\
	{														\
		if (RECORD_MSG) record_msg_write(ptr);				\
		return mqf_write(&_name, (void *)ptr);				\
	}


#define LFMQUEUE_DEF_C(_name, _type,_num, _sil) 					\
	_type buf##_name[_num] MQF_ATTRIB;									\
    mqf_t _name = {.head=0, .tail=0, .msgsiz=sizeof(_type), .num=_num, .maxuse=0, .msgbuf=(uint8_t *) buf##_name, .silentdrop=_sil} ;


void mqf_clear(mqf_t *);

int mqf_read(mqf_t *, void *);

int mqf_write(mqf_t *, void *);

int mqf_len(const mqf_t *m);

#endif /* UTILS_LF_MQUEUE_H_ */
