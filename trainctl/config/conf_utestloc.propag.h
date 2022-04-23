// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_utestloc_propag_H_
#define _conf_utestloc_propag_H_

#include <stdint.h>
// utestloc for propag


int conf_utestloc_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_utestloc_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_utestloc 9

#define conf_numfield_utestloc_alpha 		0
#define conf_numfield_utestloc_beta 		1


void *conf_utestloc_ptr(void);
int32_t conf_utestloc_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_utestloc_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
