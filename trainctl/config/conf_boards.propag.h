// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_boards_propag_H_
#define _conf_boards_propag_H_

#include <stdint.h>
// boards for propag


int conf_boards_propagate(unsigned int numinst, unsigned int numfield, int32_t value);


int32_t conf_boards_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);

#define conf_lnum_boards 11

#define conf_numfield_boards_uuid 		0
#define conf_numfield_boards_board_type 		1
#define conf_numfield_boards_disable 		2


void *conf_boards_ptr(void);
int32_t conf_boards_local_get(unsigned int fieldnum, unsigned int instnum);
void conf_boards_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);



#endif
