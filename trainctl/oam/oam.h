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

/*
* file:    4bit
* board:   4 bits
* inst:    6 bits
* field:   5 bits
* value : 21 bits
*/

void oam_decode_val40(uint64_t  val40, unsigned int *fnum, unsigned int *brd, unsigned int *inst, unsigned int *field, int32_t *v);
void oam_encode_val40(uint64_t *val40, unsigned int  fnum, unsigned int  brd, unsigned int  inst, unsigned int  field, int32_t  v);

#endif /* OAM_OAM_H_ */
