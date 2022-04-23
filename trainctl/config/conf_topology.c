// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_topology.h"
#include "conf_topology.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif



#include "../msg/trainmsg.h"

#define _PTS(pi, ...)  ,pi,{__VA_ARGS__}
#define _VP {-1, -1}
#define L0 (2)


#define FUTURE_CANTON 0xFF



#ifdef TRN_BOARD_MAIN

unsigned int conf_topology_num_entries(void)
{
    return 64; // 64 
}

static conf_topology_t conf_topology[64] = {
  {     // 0
     .canton_addr = CNUM(0, 0),
     .ina_segnum = 2,
     .steep = 0,
     .length_cm = 98,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = 1,
     .right2 = -1,
     .rtn = 0,
     .p0 = 2,
     .points = {{L0+1,2}, {L0+4,2}, {L0+5,3}, {L0+5,9}},
  },
  {     // 1
     .canton_addr = CNUM(0, 1),
     .ina_segnum = 1,
     .steep = 0,
     .length_cm = 23,
     .left1 = 0,
     .left2 = 3,
     .ltn = 0,
     .right1 = 4,
     .right2 = 12,
     .rtn = 5,
     .p0 = 0,
     .points = {{L0+5,10}, {L0+5,11}, _VP, _VP},
  },
  {     // 2
     .canton_addr = CNUM(0, 2),
     .ina_segnum = 0,
     .steep = 0,
     .length_cm = 70,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = 3,
     .right2 = 11,
     .rtn = 4,
     .p0 = 2,
     .points = {{L0+1,3}, {L0+3,3}, {L0+4,4}, {L0+4,5}},
  },
  {     // 3
     .canton_addr = CNUM(0, 2),
     .ina_segnum = 0,
     .steep = 0,
     .length_cm = 20,
     .left1 = 2,
     .left2 = -1,
     .ltn = 4,
     .right1 = -1,
     .right2 = 1,
     .rtn = 0,
     .p0 = 0,
     .points = {{L0+4,6}, {L0+4, 9}, _VP, _VP},
  },
  {     // 4
     .canton_addr = CNUM(0, 1),
     .ina_segnum = 1,
     .steep = 0,
     .length_cm = 22,
     .left1 = 1,
     .left2 = -1,
     .ltn = 5,
     .right1 = -1,
     .right2 = 5,
     .rtn = 1,
     .p0 = 0,
     .points = {{L0+5, 12}, {L0+5, 14}, _VP, _VP},
  },
  {     // 5
     .canton_addr = CNUM(0, 3),
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 54,
     .left1 = 6,
     .left2 = 4,
     .ltn = 1,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+6,15}, {L0+6,17}, {L0+5,18}, {L0+1,18}},
  },
  {     // 6
     .canton_addr = CNUM(0, 3),
     .ina_segnum = 0xFF,
     .steep = -1,
     .length_cm = 80,
     .left1 = -1,
     .left2 = 7,
     .ltn = 2,
     .right1 = 5,
     .right2 = -1,
     .rtn = 1,
     .p0 = 0,
     .points = {{L0+7,4}, {L0+7,8}, {L0+6,10}, {L0+6,14}},
  },
  {     // 7
     .canton_addr = CNUM(0, 3),
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 58,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = 8,
     .right2 = 6,
     .rtn = 2,
     .p0 = 0,
     .points = {{L0+1,1}, {L0+5,1}, {L0+6,2}, {L0+6,3}},
  },
  {     // 8
     .canton_addr = CNUM(0, 3),
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 36,
     .left1 = 7,
     .left2 = -1,
     .ltn = 2,
     .right1 = 9,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+6,4}, {L0+6,6}, _VP, _VP},
  },
  {     // 9
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 60,
     .left1 = 8,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = 10,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 3,
     .points = {{L0+6,6}, {L0+6,8}, {L0+7,10} ,{L0+7, 14} },
  },
  {     // 10
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 60,
     .left1 = 9,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+7,14}, {L0+7,18}, {L0+6,19}, {L0+1, 19} },
  },
  {     // 11
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = -1,
     .left2 = 2,
     .ltn = 4,
     .right1 = -1,
     .right2 = 21,
     .rtn = 11,
     .p0 = 0,
     .points = {{L0+3,6}, {L0+3,7}, _VP, _VP },
  },
  {     // 12
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = -1,
     .left2 = 1,
     .ltn = 5,
     .right1 = 13,
     .right2 = -1,
     .rtn = 6,
     .p0 = 0,
     .points = {{L0+4,12}, {L0+3, 13}, _VP, _VP },
  },
  {     // 13
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = 12,
     .left2 = 15,
     .ltn = 6,
     .right1 = 14,
     .right2 = -1,
     .rtn = 7,
     .p0 = 0,
     .points = {{L0+2,14}, {L0+1, 15}, _VP, _VP },
  },
  {     // 14
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = 13,
     .left2 = 16,
     .ltn = 7,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+0,16}, {L0-1, 17}, _VP, _VP },
  },
  {     // 15
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = 21,
     .left2 = -1,
     .ltn = 10,
     .right1 = -1,
     .right2 = -1,
     .rtn = 6,
     .p0 = 0,
     .points = {{L0+2,10}, {L0+2, 13}, _VP, _VP },
  },
  {     // 16
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = 17,
     .left2 = 19,
     .ltn = 8,
     .right1 = -1,
     .right2 = 14,
     .rtn = 7,
     .p0 = 0,
     .points = {{L0+0,14}, {L0+0, 15}, _VP, _VP },
  },
  {     // 17
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = 18,
     .left2 = 20,
     .ltn = 9,
     .right1 = 16,
     .right2 = -1,
     .rtn = 8,
     .p0 = 0,
     .points = {{L0+0, 12}, {L0+0, 13}, _VP, _VP },
  },
  {     // 18
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = 17,
     .right2 = -1,
     .rtn = 9,
     .p0 = 0,
     .points = {{L0+0, 4}, {L0+0, 11}, _VP, _VP},
  },
  {     // 19
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = 16,
     .rtn = 8,
     .p0 = 0,
     .points = {{L0-1,4}, {L0-1, 13}, _VP, _VP},
  },
  {     // 20
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = -1,
     .left2 = 21,
     .ltn = 10,
     .right1 = -1,
     .right2 = 17,
     .rtn = 9,
     .p0 = 0,
     .points = {{L0+1,10}, {L0+1, 11}, _VP, _VP},
  },
  {     // 21
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = 22,
     .left2 = 11,
     .ltn = 11,
     .right1 = 20,
     .right2 = 15,
     .rtn = 10,
     .p0 = 0,
     .points = {{L0+2,8}, {L0+2, 9}, _VP, _VP},
  },
  {     // 22
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 20,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = 21,
     .right2 = -1,
     .rtn = 11,
     .p0 = 0,
     .points = {{L0+2,4}, {L0+2, 7}, _VP, _VP},
  },
  {     // 23
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 24
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 25
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 26
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 27
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 28
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 29
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 30
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 31
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 32
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 33
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 34
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 35
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 36
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 37
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 38
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 39
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 40
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 41
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 42
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 43
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 44
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 45
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 46
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 47
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 48
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 49
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 50
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 51
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 52
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 53
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 54
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 55
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 56
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 57
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 58
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 59
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 60
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 61
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 62
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  },
  {     // 63
     .canton_addr = FUTURE_CANTON,
     .ina_segnum = 0xFF,
     .steep = 0,
     .length_cm = 0,
     .left1 = -1,
     .left2 = -1,
     .ltn = 0xFF,
     .right1 = -1,
     .right2 = -1,
     .rtn = 0xFF,
     .p0 = 0,
     .points = {{L0+2,0}, {L0+0, 0}, _VP, _VP},
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_topology_num_entries(void)
{
    return 0; // 0 
}

static conf_topology_t conf_topology[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_topology_num_entries(void)
{
    return 0; // 0 
}

static conf_topology_t conf_topology[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_topology_num_entries(void)
{
    return 0; // 0 
}

static conf_topology_t conf_topology[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_topology_t *conf_topology_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_topology_num_entries()) {
        return NULL;
    }
    return &conf_topology[num];
}



void *conf_topology_ptr(void)
{
    return &conf_topology[0];
}



int32_t conf_topology_local_get(unsigned int fieldnum, unsigned int instnum)
{
    const conf_topology_t *c = conf_topology_get(instnum);
    if (!c) return 0;
    switch (fieldnum) {
    default: break;
    case conf_numfield_topology_canton_addr:
        return c->canton_addr;
    case conf_numfield_topology_ina_segnum:
        return c->ina_segnum;
    case conf_numfield_topology_steep:
        return c->steep;
    case conf_numfield_topology_length_cm:
        return c->length_cm;
    case conf_numfield_topology_left1:
        return c->left1;
    case conf_numfield_topology_left2:
        return c->left2;
    case conf_numfield_topology_ltn:
        return c->ltn;
    case conf_numfield_topology_right1:
        return c->right1;
    case conf_numfield_topology_right2:
        return c->right2;
    case conf_numfield_topology_rtn:
        return c->rtn;
    case conf_numfield_topology_p0:
        return c->p0;
    }
    return 0;
}



void conf_topology_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_topology_t *ca = (conf_topology_t *) conf_topology_ptr();
    if (!ca) return;
    conf_topology_t *c = &ca[instnum];
    switch (fieldnum) {
    default: break;
    case conf_numfield_topology_canton_addr:
        c->canton_addr = v;
        break;
    case conf_numfield_topology_ina_segnum:
        c->ina_segnum = v;
        break;
    case conf_numfield_topology_steep:
        c->steep = v;
        break;
    case conf_numfield_topology_length_cm:
        c->length_cm = v;
        break;
    case conf_numfield_topology_left1:
        c->left1 = v;
        break;
    case conf_numfield_topology_left2:
        c->left2 = v;
        break;
    case conf_numfield_topology_ltn:
        c->ltn = v;
        break;
    case conf_numfield_topology_right1:
        c->right1 = v;
        break;
    case conf_numfield_topology_right2:
        c->right2 = v;
        break;
    case conf_numfield_topology_rtn:
        c->rtn = v;
        break;
    case conf_numfield_topology_p0:
        c->p0 = v;
        break;
    }

}

// topology config store type 1 num 12



// end
