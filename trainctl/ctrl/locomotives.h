//
//  locomotives.h
//  train_throttle
//
//  Created by Daniel Braun on 31/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#ifndef locomotives_h
#define locomotives_h

typedef enum {
    loco_unknown = 0,
    Marklin8805_BR29,       // 1
    Marklin8821_V200,       // 2
    Marklin8895_BR74,       // 3
    Marklin8875_V160,       // 4
    
    NumKnownLoco            // last in enum
}  __attribute((packed)) locomotive_t;

extern const char *locomotiveNames[NumKnownLoco];

#endif /* locomotives_h */
