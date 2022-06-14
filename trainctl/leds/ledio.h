//
//  ledio.h
//  train_throttle
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef ledio_h
#define ledio_h

#ifdef TRAIN_SIMU
#include <stdio.h>
#endif

#include <stdint.h>

void led_io(uint8_t lednum, uint8_t v);

#endif /* ledio_h */
