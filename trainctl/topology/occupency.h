//
//  occupency.h
//  train_throttle
//
//  Created by Daniel BRAUN on 22/04/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef TOPOLOGY_OCCUPENCY_H_
#define TOPOLOGY_OCCUPENCY_H_



#define USE_BLOCK_DELAY_FREE 0



#define BLK_OCC_FREE        0x00
//#define BLK_OCC_STOP        0x01
#define BLK_OCC_LOCO_STOP   0x01
#define BLK_OCC_LOCO_LEFT   0x02
#define BLK_OCC_LOCO_RIGHT  0x03
#define BLK_OCC_C2          0x04

#define BLK_OCC_CARS        0x05        // #longtrain

#define BLK_OCC_DELAY1      0x10
#define BLK_OCC_DELAYM      0x16

extern uint8_t notify_occupency_change;

void occupency_clear(void);


// obsolete
void set_block_addr_occupency(xblkaddr_t blkaddr, uint8_t v, uint8_t trnum, lsblk_num_t lsb);

/// occupency_set_occupied
/// set block as occupied by locomomtive
/// @param blkaddr block addr
/// @param trnum train number
/// @param lsb concerned lsb
int occupency_set_occupied(xblkaddr_t blkaddr, uint8_t trnum, lsblk_num_t lsb, int sdir);


/// occupency_set_occupied_car
/// set block as occupied by left or right cars
/// returns -1 if failure (block already occupied)
/// @param blkaddr block addr
/// @param trnum train number
/// @param lsb concern lsb
int occupency_set_occupied_car(xblkaddr_t blkaddr, uint8_t trnum, lsblk_num_t lsb, int sdir);

/// occupency_set_free
/// clear occupency on block
/// @param blkadrr block addr
/// @param trnum train number or -1
void occupency_set_free(xblkaddr_t blkadrr, uint8_t trnum);


uint8_t get_block_addr_occupency(xblkaddr_t blkaddr);
uint8_t occupency_block_addr_info(xblkaddr_t blkaddr, uint8_t *ptrn, uint8_t *psblk);
void check_block_delayed(uint32_t tick, uint32_t dt);

uint8_t occupency_block_is_free(xblkaddr_t blkaddr, uint8_t trnum);




static inline uint8_t occupied(int dir)
{
    if (dir<0) return BLK_OCC_LOCO_LEFT;
    if (dir>0) return BLK_OCC_LOCO_RIGHT;
    return BLK_OCC_LOCO_STOP;
}


int occupency_turnout_reserve(xtrnaddr_t turnout, int8_t train);
void occupency_turnout_release(xtrnaddr_t turnout, int8_t train);


#endif /* occupency_h */
