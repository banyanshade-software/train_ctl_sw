//
//  topologyP.h
//  train_throttle
//
//  Created by Daniel BRAUN on 06/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef topologyP_h
#define topologyP_h

//#define TOPOLOGY_SVG

#ifdef TOPOLOGY_SVG
typedef struct {
    int l;
    int c;
} coord_t;
#define MAX_POINTS 4

#endif

typedef struct {
    uint8_t canton_addr;
    uint8_t ina_segnum;
    
    uint8_t length_cm;
    
    int8_t left1;
    int8_t left2;
    uint8_t ltn; // leeee turnout
    int8_t right1;
    int8_t right2;
    uint8_t rtn; // leeee turnout
    
#ifdef TOPOLOGY_SVG
    coord_t points[MAX_POINTS];
#endif
} topo_lsblk_t;


const topo_lsblk_t *topology_get_sblkd(int lsblk);
const int topology_num_sblkd(void);

#endif /* topologyP_h */
