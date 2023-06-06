//
//  spdcxdir.h
//  train_throttle
//
//  Created by Daniel Braun on 23/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef spdcxdir_h
#define spdcxdir_h

static inline int __spdcx_dir(uint8_t field, int n)
{
    uint8_t bb = (field >> (2*n)) & 0x3;
    switch (bb) {
    case 0xFF:
        return 0;
    case 0x00:
        return 0;
        break;
    case 0x01:
        return 1;
    case 0x02:
        return -1;
    default:
        return 0;
        break;
    }
}

static inline uint8_t __spdcx_bit(int n, int sdir)
{
    uint8_t b = 0;
    if (sdir > 0) {
        b = 0x01;
    } else if (sdir < 0) {
        b = 0x02;
    }
    return b << (n*2);
}
#endif /* spdcxdir_h */
