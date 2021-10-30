//
//  occupency.h
//  train_throttle
//
//  Created by Daniel BRAUN on 22/04/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef TOPOLOGY_OCCUPENCY_H_
#define TOPOLOGY_OCCUPENCY_H_



#define USE_BLOCK_DELAY_FREE 1



#define BLK_OCC_FREE    0x00
#define BLK_OCC_STOP    0x01
#define BLK_OCC_LEFT    0x02
#define BLK_OCC_RIGHT    0x03
#define BLK_OCC_C2        0x04

#define BLK_OCC_DELAY1    0x10
#define BLK_OCC_DELAYM    0x16

extern uint8_t notify_occupency_change;

void occupency_clear(void);

void set_block_addr_occupency(uint8_t blkaddr, uint8_t v, uint8_t trnum, lsblk_num_t lsb);
uint8_t get_block_addr_occupency(uint8_t blknum);
void check_block_delayed(uint32_t tick);

extern uint8_t occupency_changed;


static inline uint8_t occupied(int dir)
{
    if (dir<0) return BLK_OCC_LEFT;
    if (dir>0) return BLK_OCC_RIGHT;
    return BLK_OCC_STOP;
}
#endif /* occupency_h */
