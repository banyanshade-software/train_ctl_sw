/*
 * cantask.h
 *
 *  Created on: Nov 22, 2021
 *      Author: danielbraun
 */

#ifndef STM32_CANMSG_H_
#define STM32_CANMSG_H_



#include "cmsis_os.h"

#include "../trainctl_iface.h"


void CAN_Tasklet(uint32_t notif_flags, uint32_t tick, uint32_t dt);

#endif /* STM32_CANMSG_H_ */
