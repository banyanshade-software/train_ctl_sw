// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_train_propag_H_
#define _conf_train_propag_H_

#include <stdint.h>
// train for propag


int conf_train_propagate(int numinst, int numfield, int32_t value);

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
#define conf_numfield_fix_bemf 		9
#define conf_numfield_en_spd2pow 		10
#define conf_numfield_reversed 		11
#define conf_numfield_min_power 		12
#define conf_numfield_notify_pose 		13
#define conf_numfield_bemfIIR 		14
#define conf_numfield_postIIR 		15
#define conf_numfield_slipping 		16
#define conf_numfield_pose_per_cm 		17
#define conf_numfield_trainlen_left 		18
#define conf_numfield_trainlen_right 		19



#endif
