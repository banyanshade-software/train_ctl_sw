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
	uint16_t msgsiz;
	uint16_t num;
	uint8_t *msgbuf;
} mqf_t;

#define LFMQUEUE_DEF_H(_name, _type) 						\
    extern mqf_t _name; 									\
    														\
	static inline int mqf_read_ ## _name(_type *ptr)			\
	{														\
		return mqf_read(&_name, ptr);						\
	}														\
	static inline int mqf_write_ ## _name(_type *ptr)		\
	{														\
		return mqf_write(&_name, (void *)ptr);				\
	}


#define LFMQUEUE_DEF_C(_name, _type,_num) 					\
	_type buf##_name[_num];									\
    mqf_t _name = {0, 0, sizeof(_type), _num, (uint8_t *) buf##_name};


void mqf_clear(mqf_t *);

int mqf_read(mqf_t *, void *);

int mqf_write(mqf_t *, void *);

#endif /* UTILS_LF_MQUEUE_H_ */
