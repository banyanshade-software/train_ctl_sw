/*
 * msgrecord.c
 *
 *  Created on: Aug 21, 2022
 *      Author: danielbraun
 */


#include "../misc.h"
#include "trainmsg.h"
#include "msgrecord.h"
#include "trainmsgstr.h"

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
	//endidx = 0;
	//startidx = 0;
	disable_record = 0;
}
#ifdef TRAIN_SIMU


const char *addrname(uint8_t a)
{
    static char r[32];
    if ((0)) {
    } else if (MA1_IS_CTRL(a)) {
        sprintf(r, "MA1_CTRL(%d)",   a & 0xF);
    } else if (MA1_IS_SPDCTL(a)) {
        sprintf(r, "MA1_SPDCTL(%d)", a & 0xF);
    } else if (MA0_IS_CANTON(a)) {
        sprintf(r, "MA0_CANTON(%d)", a & 0xF);
    } else {
        sprintf(r, "%2.2X", a);
    }
    return r;
}

static void dumpastext(const uint8_t *d, int l)
{
    msgrecord_t *r = (msgrecord_t *)d;
    printf("{%d, %u, {", r->dir, r->ts);
    printf(".from=0x%s, ", addrname(r->m.from));
    printf(".to=0x%s, ", addrname(r->m.to));
    printf(".subc=%d, .cmd=%s, ",
           r->m.subc, traincmd_name(r->m.cmd));
    msg_type_t f = traincmd_format(r->m.cmd);
    switch (f) {
        case CMD_TYPE_V32:
            printf(".v32=%d", r->m.v32);
            break;
        case CMD_TYPE_B4:
            printf(".vb0=%d,.vb1=%d,.vb2=%d,.vb3=%d",
                   r->m.vb0, r->m.vb1, r->m.vb2, r->m.vb3);
            break;
        case CMD_TYPE_V40:
            printf(".val40=%llX", r->m.val40);
            break;
        case CMD_TYPE_VCU:
            printf(".v16=%d, .vb8=%d, .vcu8=%d", r->m.va16, r->m.vb8, r->m.vcu8);
            break;
        case CMD_TYPE_V16:
            printf(".v1=%d, .v2=%d", r->m.v1, r->m.v2);
            break;
        default:
            break;
    }
    printf("}},\n");
}
void record_msg_dump(void)
{
    frame_send_recordmsg(dumpastext);
}
#endif
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
