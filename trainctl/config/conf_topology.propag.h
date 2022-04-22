// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_topology_propag_H_
#define _conf_topology_propag_H_

#include <stdint.h>
// topology for propag


int conf_topology_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_topology_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_topology 12

#define conf_numfield_topology_canton_addr 		0
#define conf_numfield_topology_ina_segnum 		1
#define conf_numfield_topology_steep 		2
#define conf_numfield_topology_length_cm 		3
#define conf_numfield_topology_left1 		4
#define conf_numfield_topology_left2 		5
#define conf_numfield_topology_ltn 		6
#define conf_numfield_topology_right1 		7
#define conf_numfield_topology_right2 		8
#define conf_numfield_topology_rtn 		9
#define conf_numfield_topology_p0 		10
#define conf_numfield_topology_points 		11


void *conf_topology_ptr(void);
int32_t conf_topology_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_topology_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
