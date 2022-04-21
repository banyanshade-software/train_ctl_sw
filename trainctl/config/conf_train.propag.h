// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_train_propag_H_
#define _conf_train_propag_H_

#include <stdint.h>
// train for propag


int conf_train_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_train_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_pnum_train 1

#define conf_numfield_kP 		0
#define conf_numfield_kI 		1
#define conf_numfield_kD 		2
#define conf_numfield_dec 		3
#define conf_numfield_acc 		4
#define conf_numfield_volt_policy 		5
#define conf_numfield_enabled 		6
#define conf_numfield_enable_inertia 		7
#define conf_numfield_enable_pid 		8
#define conf_numfield_reversed 		9
#define conf_numfield_bemfIIR 		10
#define conf_numfield_postIIR 		11
#define conf_numfield_slipping 		12
#define conf_numfield_pose_per_cm 		13
#define conf_numfield_trainlen_left 		14
#define conf_numfield_trainlen_right 		15



#endif
