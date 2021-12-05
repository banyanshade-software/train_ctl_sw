//
//  misc.h
//  trackplan2
//
//  Created by Daniel BRAUN on 04/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef misc_h
#define misc_h

#include <stdint.h>

#define _UNUSED_ __attribute__((unused))

static inline void Error_Handler(void) {
    for (;;) {
    }
}

#define NUM_TURNOUTS 16
//#define DBG_TURNOUTS 1
#define DBG_CTRL 1
#define itm_debug3(_fl, _msg, _a, _b, _c) do {printf(_msg  "%d %d %d", _a, _b, _c);} while(0)
#define itm_debug2(_fl, _msg, _a, _b) do {printf(_msg  "%d %d", _a, _b);} while(0)
#define itm_debug1(_fl, _msg, _a) do {printf(_msg  "%d", _a);} while(0)

extern uint32_t tick;

#endif /* misc_h */
