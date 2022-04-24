/*
 * usbtask.h
 *
 *  Created on: 23 avr. 2022
 *      Author: danielbraun
 */

#ifndef STM32_USBTASK_H_
#define STM32_USBTASK_H_

void StartUsbTask(const void *arg);
void USB_Tasklet(uint32_t notif_flags, uint32_t tick, uint32_t dt);

int can_send_stat(void);

#endif /* STM32_USBTASK_H_ */
