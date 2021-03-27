//
//  trkplan.h
//  trackplan
//
//  Created by Daniel BRAUN on 10/03/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef trkplan_h
#define trkplan_h

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_T       8
#define MAX_TRAINS  4
#define MAX_SEGM    16
/*
 per train:
 X X            motion   01=<-  11=-> 00=idle  (10=unused)
      X         next-dir
 16bit contains up to 5 trains, MSB not used
 */
static inline uint16_t trbits_gettrain(uint16_t bits, int8_t ntrain)
{
    uint16_t v = (bits >> (3*ntrain)) & 0x7;
    return v;
}
static inline int16_t trbits_motion(int16_t bits)
{
    int16_t t = (bits & 0x6)>>1;
    switch (t) {
        case 0: return 0;
        case 1: return 1;
        case 3: return -1;
        default: abort();
            break;
    }
    return 0;
}
static inline uint16_t trbits_nextdir(int16_t bits)
{
    return bits & 0x01;
}

static inline void set_motion_dir(uint16_t *pbits, int trn, int motion, int nextdir)
{
    uint16_t t = ((uint16_t)(motion & 0x3))<<1 | (nextdir & 1);
    uint16_t m = 0x7;
    t = t<<(trn*3);
    m = m<<(trn*3);
    *pbits &= ~m;
    *pbits |= t;
    
    if ((1)) {
        int m = trbits_motion(trbits_gettrain(*pbits, trn));
        int d = trbits_nextdir(trbits_gettrain(*pbits, trn));
        if (m != motion) abort();
        if (d != nextdir) abort();
    }
}

typedef  struct {
    uint16_t step[MAX_T];
    int16_t score;
    uint8_t mark;
} tplan_t;
// ------------------------------------------------------------------

typedef struct {
    uint8_t left_1;
    uint8_t left_2;
    uint8_t right_1;
    uint8_t right_2;
} track_segment_t;


// ------------------------------------------------------------------

typedef struct {
    int8_t s[MAX_SEGM];
    // more ?
} segstate_t;

typedef struct {
    int8_t t[MAX_TRAINS];
    uint8_t flags;
    int16_t score;
} trstate_t;


#define RC_OK  0
#define RC_OUT 1
#define RC_COL 2

int get_segstate(trstate_t *st, segstate_t *retseg);
int update_state(trstate_t *st, track_segment_t *trseg, uint16_t step);
int update_state_all(trstate_t *st, track_segment_t *trseg, tplan_t *p);

void test_me(void);

#endif /* trkplan_h */
