/*
 * ina3221.h
 *
 *  Created on: Apr 13, 2021
 *      Author: danielbraun
 */

#ifndef INA3221_INA3221_H_
#define INA3221_INA3221_H_



void ina3221_init(void);
void ina3221_start_read(void);
extern int16_t ina3221_values[3];

extern uint32_t ina3221_nscan;
extern uint32_t ina3221_scan_dur;
extern uint32_t ina3221_inter_dur;

#endif /* INA3221_INA3221_H_ */
