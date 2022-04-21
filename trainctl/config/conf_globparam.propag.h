// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_globparam_propag_H_
#define _conf_globparam_propag_H_

#include <stdint.h>
// globparam for propag


int conf_globparam_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_globparam_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_pnum_globparam 10

#define conf_numfield_pwmfreq 		0
#define conf_numfield_test_mode 		1
#define conf_numfield_oscilo 		2



#endif
