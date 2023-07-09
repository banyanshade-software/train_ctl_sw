//
//  detectP.h
//  train_throttle
//
//  Created by Daniel BRAUN on 21/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef detectP_h
#define detectP_h

void detect2_init(void);

void detect2_process_tick(uint32_t tick, uint32_t dt);

void detect2_process_msg(msg_64_t *m);


#endif /* detectP_h */
