//
//  AppDelegateP.h
//  train_throttle
//
//  Created by Daniel Braun on 27/09/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef AppDelegateP_h
#define AppDelegateP_h

#import "train_simu.h"
#import "trainctl_iface.h"

@interface AppDelegate()


- (void) sendMsg64:(msg_64_t)m;


@end

extern AppDelegate *theDelegate;

#endif /* AppDelegateP_h */
