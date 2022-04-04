/*
 * cantest.h
 *
 *  Created on: 2 avr. 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_H_
#define OAM_OAM_H_


void OAM_Tasklet(uint32_t notif_flags, uint32_t tick, uint32_t dt);

uint32_t getDeviceId(void);

#endif /* OAM_OAM_H_ */
