//
//  trainmsgstr.h
//  train_throttle
//
//  Created by Daniel Braun on 20/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef trainmsgstr_h
#define trainmsgstr_h

#include <stdint.h>

typedef enum {
    CMD_TYPE_V32,
    CMD_TYPE_B4,
    CMD_TYPE_V40,
    CMD_TYPE_VCU,
    CMD_TYPE_V16,
} msg_type_t;

const char *traincmd_name(uint8_t cmd);
msg_type_t traincmd_format(uint8_t cmd);

#endif /* trainmsgstr_h */
