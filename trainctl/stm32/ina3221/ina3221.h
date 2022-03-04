/*
 * ina3221.h
 *
 *  Created on: Apr 13, 2021
 *      Author: danielbraun
 */

#ifndef INA3221_INA3221_H_
#define INA3221_INA3221_H_


/*
void ina3221_init(void);
void ina3221_start_read(void);
extern int16_t ina3221_values[3];

extern uint32_t ina3221_nscan;
extern uint32_t ina3221_scan_dur;
extern uint32_t ina3221_inter_dur;
*/

// normally upper layer dont need to know this
// but ina3221_devices array (contains 1 if device detected, 0 otherwise)
// is made public so we can display/check presence by IHM


extern uint8_t ina3221_devices[4];

#define INA3221_NUM_DEVICES 4
#define INA3221_NUM_VALS (INA3221_NUM_DEVICES * 3)



extern volatile uint16_t *cur_values;
extern volatile uint16_t *prev_values;



#endif /* INA3221_INA3221_H_ */
