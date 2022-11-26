//
//  bitarray.h
//  train_throttle
//
//  Created by Daniel Braun on 11/11/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef bitarray_h
#define bitarray_h

#define DECL_BIT_ARRAY(_name, _nbits) \
    uint32_t _name[(_nbits+31)/32]

#define BIT_ARRAY_CLEAR(_name) do {\
    memset(_name, 0, sizeof(_name)); \
} while(0)

#define BIT_ARRAY_SET(_name, _n) do {\
    _name[(_n)/32] |= 1UL << ((_n) % 32); \
} while(0)

#define BIT_ARRAY_VALUE(_name, _n) \
    (( _name[(_n)/32] & (1UL << ((_n) % 32)) ) ? 1 : 0)

#endif /* bitarray_h */
