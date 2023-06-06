// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_train_propag_H_
#define _conf_train_propag_H_

#include <stdint.h>
// train for propag


int conf_train_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_train_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_train 1

#define conf_numfield_pidctl_kP 		0
#define conf_numfield_pidctl_kI 		1
#define conf_numfield_pidctl_kD 		2
#define conf_numfield_inertia_dec 		3
#define conf_numfield_inertia_acc 		4
#define conf_numfield_train_volt_policy 		5
#define conf_numfield_train_enabled 		6
#define conf_numfield_train_enable_inertia 		7
#define conf_numfield_train_enable_pid 		8
#define conf_numfield_train_reversed 		9
#define conf_numfield_train_bemfIIR 		10
#define conf_numfield_train_postIIR 		11
#define conf_numfield_train_slipping 		12
#define conf_numfield_train_pose_per_cm 		13
#define conf_numfield_train_trainlen_left_cm 		14
#define conf_numfield_train_trainlen_right_cm 		15


void *conf_train_ptr(void);
int32_t conf_train_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_train_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
