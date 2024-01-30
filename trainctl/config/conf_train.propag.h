// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_train_propag_H_
#define _conf_train_propag_H_

#include <stdint.h>
// train for propag


int conf_train_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_train_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_train 1

#define conf_numfield_train_locotype 		0
#define conf_numfield_train_enabled 		1
#define conf_numfield_train_trainlen_left_cm 		2
#define conf_numfield_train_trainlen_right_cm 		3


void *conf_train_ptr(void);
int32_t conf_train_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_train_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
