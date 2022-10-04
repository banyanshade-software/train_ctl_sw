/*
 * msgrecord.c
 *
 *  Created on: Aug 21, 2022
 *      Author: danielbraun
 */


#include "../misc.h"
#include "trainmsg.h"
#include "msgrecord.h"

//#define MSGRECORD_NUMREC 0		// define to 0 to disable recording
#define MSGRECORD_NUMREC 512

#define MSGRECORD_CIRCULAR 1

#if MSGRECORD_NUMREC

static msgrecord_t recordbuf[MSGRECORD_NUMREC] = {0};
static uint16_t startidx = 0;
static uint16_t endidx = 0;

/*
 * recording should be enabled for a single tasklet at a time
 * (and therefore there is no thread issue here, except on disable_record
 */

static volatile uint32_t disable_record = 0; // recording disabled during sending

static msgrecord_t *next_record_buf(void)
{
	if (disable_record) return NULL;

	msgrecord_t *r;
	if (!MSGRECORD_CIRCULAR) {
		// non circular mode, stop when buffer is full
		if (endidx == (MSGRECORD_NUMREC)) return NULL;
		r = &recordbuf[endidx];
		endidx++;
		return r;
	} else {
		// circular buffer mode
		r = &recordbuf[endidx];
		endidx = (endidx+1) % (MSGRECORD_NUMREC);
		if (endidx == startidx) {
			startidx = (startidx+1) % (MSGRECORD_NUMREC);
		}
		return r;
	}
}
void record_msg_read(void *ptr)
{
	msgrecord_t *rec = next_record_buf();
	if (!rec) return;
	memcpy(&rec->m, ptr, sizeof(msg_64_t));
	rec->dir = 0;
	rec->ts = HAL_GetTick();
}
void record_msg_write(void *ptr)
{
	msgrecord_t *rec = next_record_buf();
	if (!rec) return;
	memcpy(&rec->m, ptr, sizeof(msg_64_t));
	rec->dir = 1;
	rec->ts = HAL_GetTick();
}

void frame_send_recordmsg(void(*cb)(const uint8_t *d, int l))
{
	disable_record = 1;
	int idx = startidx;
	for (;;) {
		if (idx == endidx) break;
		msgrecord_t *r = &recordbuf[idx];
		cb((uint8_t *)r, sizeof(msgrecord_t));
		idx = (idx + 1) % (MSGRECORD_NUMREC);
	}
	endidx = 0;
	startidx = 0;
	disable_record = 0;
}

#else
// disabled


void record_msg_read(void *ptr)
{
	(void) ptr; // unused
}
void record_msg_write(void *ptr)
{
	(void) ptr; //unused
}

void frame_send_recordmsg(_UNUSED_ void(*cb)(const uint8_t *d, int l))
{

}

#endif