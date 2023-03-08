/*
 * monitoring.h
 *
 *  Created on: 26 févr. 2023
 *      Author: danielbraun
 */

#ifndef STM32_MONITORING_H_
#define STM32_MONITORING_H_

#include "monitoring_def.h"


//#define ENABLE_TRACES

#ifdef ENABLE_TRACES
#define MONITOR_CH 1
#define TRACE_MAW_WAIT 50

// pxTaskTag

extern void itmTraceSend(int ch, uint8_t type,  uint8_t data);
void itmTraceTaskCreate(int tcbnnum, int tasknum, char *taskname);

#define traceTASK_SWITCHED_IN()      		do { itmTraceSend(MONITOR_CH, MONITOR_SW_IN, pxCurrentTCB->uxTCBNumber); } while(0)
#define traceTASK_SWITCHED_OUT()     		do { itmTraceSend(MONITOR_CH, MONITOR_SW_OUT, pxCurrentTCB->uxTCBNumber); } while(0)

#define traceTASK_INCREMENT_TICK(t)  		do { itmTraceSend(MONITOR_CH, MONITOR_TICK, 0); } while(0)
#define traceMOVED_TASK_TO_READY_STATE(_tcb) do { itmTraceSend(MONITOR_CH, MONITOR_READY, (_tcb)->uxTCBNumber); } while(0)

#define traceTASK_NOTIFY_WAIT_BLOCK()     	do { itmTraceSend(MONITOR_CH, MONITOR_NOTIF_WB, pxCurrentTCB->uxTCBNumber); } while(0)
#define traceTASK_NOTIFY_WAIT()     		do { itmTraceSend(MONITOR_CH, MONITOR_NOTIF_W, pxCurrentTCB->uxTCBNumber); } while(0)
#define traceTASK_DELAY_UNTIL(_x)          	do { itmTraceSend(MONITOR_CH, MONITOR_DELAY_U, pxCurrentTCB->uxTCBNumber); } while(0)
#define traceTASK_DELAY() 		         	do { itmTraceSend(MONITOR_CH, MONITOR_DELAY, pxCurrentTCB->uxTCBNumber); } while(0)

#define traceTASK_NOTIFY()     				do { itmTraceSend(MONITOR_CH, MONITOR_NOTIF, pxCurrentTCB->uxTCBNumber); } while(0)
#define traceTASK_NOTIFY_FROM_ISR()     	do { itmTraceSend(MONITOR_CH, MONITOR_NOTIF_ISR, pxTCB->uxTCBNumber); } while(0)

#define traceUserTIMER(_numtim)				do { itmTraceSend(MONITOR_CH, MONITOR_EVT_TIM, _numtim); } while(0)
#define traceUserDMA(_info)				    do { itmTraceSend(MONITOR_CH, MONITOR_EVT_DMA, _info); } while(0)


//#define traceTASK_CREATE(_tcb)				do { itmTraceTaskCreate((_tcb) ? (_tcb)->uxTCBNumber : -1, (_tcb) ? (_tcb)->uxTaskNumber : -1, (_tcb) ? (_tcb)->pcTaskName: ""); } while(0)

#else

#define traceUserTIMER(_numtim)				do {  } while(0)
#define traceUserDMA(_info)				    do {  } while(0)


#endif // ENABLE_TRACES

#endif /* STM32_MONITORING_H_ */
