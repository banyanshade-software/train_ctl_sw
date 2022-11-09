// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_servo_propag_H_
#define _conf_servo_propag_H_

#include <stdint.h>
// servo for propag


int conf_servo_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_servo_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_pnum_servo 5

#define conf_numfield_servo_direction 		0
#define conf_numfield_servo_min 		1
#define conf_numfield_servo_max 		2
#define conf_numfield_servo_spd 		3
#define conf_numfield_servo_pos0 		4



#endif
