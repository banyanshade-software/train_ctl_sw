/*
 * cantest.h
 *
 *  Created on: 2 avr. 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_H_
#define OAM_OAM_H_

void OAM_Init(void);

void OAM_Tasklet(uint32_t notif_flags, uint32_t tick, uint32_t dt);

extern int OAM_NeedsReschedule;

uint32_t oam_getDeviceUniqueId(void); // returns unique uint32_t


int oam_isMaster(void);

/* encode / decode val40 in msg64_t (assuming same endieness !!)
 * struct {
 * file: 3-4bit										4
 * board: 3-4 bits									4
 * inst: 6 bits mini								6
 * field:5 bits (up to 20 fields in conf_train)	    5
 * total : 17 to 19bits -> 19 bits
 * counting subc : 40 bits available
 * -> value : 21 bits							   21
 */

void oam_decode_val40(uint64_t  val40, int *fnum, int *brd, int *inst, int *field, int32_t *v);
void oam_encode_val40(uint64_t *val40, int  fnum, int  brd, int  inst, int  field, int32_t  v);

#endif /* OAM_OAM_H_ */
