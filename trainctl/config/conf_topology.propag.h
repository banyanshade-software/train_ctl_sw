// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_topology_propag_H_
#define _conf_topology_propag_H_

#include <stdint.h>
// topology for propag


int conf_topology_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_topology_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_pnum_topology 12

#define conf_numfield_canton_addr 		0
#define conf_numfield_ina_segnum 		1
#define conf_numfield_steep 		2
#define conf_numfield_length_cm 		3
#define conf_numfield_left1 		4
#define conf_numfield_left2 		5
#define conf_numfield_ltn 		6
#define conf_numfield_right1 		7
#define conf_numfield_right2 		8
#define conf_numfield_rtn 		9
#define conf_numfield_p0 		10
#define conf_numfield_points 		11



#endif
