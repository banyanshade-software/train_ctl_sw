//
//  oam_detect.h
//  train_throttle
//
//  Created by Daniel Braun on 10/02/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#ifndef oam_detect_h
#define oam_detect_h

void oam_detect_reset(void);
void oam_train_detected(uint8_t canton, uint8_t lsblk, uint8_t loco, uint8_t reserved);

int oam_start_sblk_for_train(uint8_t tidx);

#endif /* oam_detect_h */
