// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_topology_H_
#define _conf_topology_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#endif



typedef struct {
    int l;
    int c;
} coord_t;



typedef struct conf_topology {
    uint8_t canton_addr;
    int8_t ina_segnum;
    int8_t steep;
    uint8_t length_cm;
    int8_t left1;
    int8_t left2;
    uint8_t ltn;
    int8_t right1;
    int8_t right2;
    uint8_t rtn;
    int p0;
    coord_t points[4];
} conf_topology_t;


int conf_topology_num_entries(void);
const conf_topology_t *conf_topology_get(int num);




#endif