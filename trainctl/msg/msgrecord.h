/*
 * msgrecord.h
 *
 *  Created on: Aug 21, 2022
 *      Author: danielbraun
 */

#ifndef MSG_MSGRECORD_H_
#define MSG_MSGRECORD_H_

// only one module should record at a given time
// by defining "#define RECORD_MSG 1" before including any header (at least before lf_mqueue.h)

extern void record_msg_read(void *ptr);
extern void record_msg_write(void *ptr);



struct msgrecord {
	msg_64_t m;
	uint32_t ts;
	uint8_t dir;
}  __attribute__((packed));

typedef struct msgrecord msgrecord_t;


// sending over usb (invoked by usbtask.c)
void frame_send_recordmsg(void(*cb)(const uint8_t *d, int l));

void record_msg_dump(void);

#endif /* MSG_MSGRECORD_H_ */
