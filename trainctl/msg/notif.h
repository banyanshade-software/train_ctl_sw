/*
 * notif.h
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */

#ifndef MSG_NOTIF_H_
#define MSG_NOTIF_H_





#define NOTIF_NEW_ADC_1		0x00000001
#define NOTIF_NEW_ADC_2		0x00000002


#define NOTIF_INA_TRIG		0x00000004
#define NOTIF_INA_READ		0x00000008
#define NOTIF_INA_WRCOMPL	0x00000010
#define NOTIF_INA_RDCOMPL	0x00000020
#define NOTIF_INA_ERR		0x00000040

#define NOTIF_SYSTICK		0x00000080

#define NOTIF_UART_TX		0x00000100

#endif /* MSG_NOTIF_H_ */
