// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_globparam_propag_H_
#define _conf_globparam_propag_H_

#include <stdint.h>
// globparam for propag


int conf_globparam_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_globparam_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_globparam 10

#define conf_numfield_globparam_pwmfreq 		0
#define conf_numfield_globparam_test_mode 		1
#define conf_numfield_globparam_oscilo 		2


void *conf_globparam_ptr(void);
int32_t conf_globparam_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_globparam_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
