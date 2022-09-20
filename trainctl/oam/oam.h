/*
 * cantest.h
 *
 *  Created on: 2 avr. 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_H_
#define OAM_OAM_H_

// void OAM_Init(void); now static
#include "../msg/tasklet.h"

extern tasklet_t OAM_tasklet;

//void OAM_Tasklet(uint32_t notif_flags, uint32_t tick, uint32_t dt);

/// OAM_NeedsReschedule global var (defined in oam..c) is set by oam.c to indicate it needs to be
/// reschedule. This is typically the case during 'multiboard' config file read : file is read parameter by parameter
/// and OAM_Tasklet would read only a single parameter at a time (triggering CAN msg sending to the concerned board)
/// tasklet needs to be reschedule 'quickly' to read next param, while it usually need only low frequency running
///
/// typ. whend OAM_NeedsReschedule=1 : schedule at 100Hz (or more)
/// otherwise : 10Hz
///
extern int OAM_NeedsReschedule;


/// returns a unique device id on 32 bits
///
/// TODO : this is a very naive 96bits->32bits hash, we need something more robust (at least a CRC)
uint32_t oam_getDeviceUniqueId(void); 



/*
* file:    4bit
* board:   4 bits
* inst:    6 bits
* field:   5 bits
* value : 21 bits
*/

void oam_decode_val40(uint64_t  val40, unsigned int *fnum, unsigned int *brd, unsigned int *inst, unsigned int *field, int32_t *v);
void oam_encode_val40(uint64_t *val40, unsigned int  fnum, unsigned int  brd, unsigned int  inst, unsigned int  field, int32_t  v);


extern volatile int Oam_Ready;

#endif /* OAM_OAM_H_ */
