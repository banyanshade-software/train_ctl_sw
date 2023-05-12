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

#include "trainctl_config.h"



typedef struct {
    int l;
    int c;
} coord_t;



typedef struct conf_topology {
    uint8_t canton_addr;
    uint8_t ina_segnum;
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


unsigned int conf_topology_num_entries(void);
const conf_topology_t *conf_topology_get(int num);



#ifdef TRN_BOARD_G4SLV1
#define NUM_TOPOLOGYS 0 // 0 
#endif // TRN_BOARD_G4SLV1



#ifdef TRN_BOARD_G4MASTER1
#define NUM_TOPOLOGYS 64 // 64 
#endif // TRN_BOARD_G4MASTER1



#ifdef TRN_BOARD_UNIT_TEST
#define NUM_TOPOLOGYS 32 // 32 
#endif // TRN_BOARD_UNIT_TEST



#ifdef TRN_BOARD_UI
#define NUM_TOPOLOGYS 0 // 0 
#endif // TRN_BOARD_UI



#ifdef TRN_BOARD_MAINV04
#define NUM_TOPOLOGYS 64 // 64 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_TOPOLOGYS 64 // 64 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_TOPOLOGYS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_TOPOLOGYS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_TOPOLOGYS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_TOPOLOGYS 64 // 64 
#endif // TRN_BOARD_SIMU


#define MAX_TOPOLOGYS 64




#endif
