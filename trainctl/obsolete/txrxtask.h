/*
 * txframe.h
 *
 *  Created on: Oct 16, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#ifndef STM32_TXRXTASK_H_
#define STM32_TXRXTASK_H_

#error obsolete

#include "cmsis_os.h"

#include "../trainctl_iface.h"

extern osMessageQueueId_t frameQueueHandle;

void StartTxFrameTask(void *argument);

void txframe_send(frame_msg_t *m, int discardable);


int8_t impl_CDC_Receive_FS(uint8_t* Buf, uint32_t *Len);

#endif /* STM32_TXRXTASK_H_ */
