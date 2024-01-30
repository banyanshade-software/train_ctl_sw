// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_locomotive_propag_H_
#define _conf_locomotive_propag_H_

#include <stdint.h>
// locomotive for propag


int conf_locomotive_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_locomotive_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_locomotive 6

#define conf_numfield_pidctl_kP 		0
#define conf_numfield_pidctl_kI 		1
#define conf_numfield_pidctl_kD 		2
#define conf_numfield_inertia_dec 		3
#define conf_numfield_inertia_acc 		4
#define conf_numfield_locomotive_volt_policy 		5
#define conf_numfield_locomotive_enable_inertia 		6
#define conf_numfield_locomotive_enable_pid 		7
#define conf_numfield_locomotive_reversed 		8
#define conf_numfield_locomotive_bemfIIR 		9
#define conf_numfield_locomotive_postIIR 		10
#define conf_numfield_locomotive_slipping 		11
#define conf_numfield_locomotive_pose_per_cm 		12


void *conf_locomotive_ptr(void);
int32_t conf_locomotive_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_locomotive_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
