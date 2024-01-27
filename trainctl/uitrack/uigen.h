//
//  uigen.h
//  train_throttle
//
//  Created by Daniel Braun on 27/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#ifndef uigen_h
#define uigen_h

#include "../topology/topology.h"

void uigen_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);



void received_ui_gen(msg_64_t *m);

#endif /* uigen_h */
