//
//  occupency.h
//  train_throttle
//
//  Created by Daniel BRAUN on 22/04/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef TOPOLOGY_OCCUPENCY_H_
#define TOPOLOGY_OCCUPENCY_H_

/* 5bits block and subblock clearance/occupency
 occupency can be OR'ed between block and subblock
 on block only, train num is in 3 lsb
 */
#define OCC_OCCUPIED        0x80
#define OCC_SLOW_LR         0x40
#define OCC_SLOW_RL         0x20
#define OCC_FORBID_LR       0x10
#define OCC_FORBID_RL       0x08

#endif /* occupency_h */
