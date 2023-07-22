//
//  SimTrain.h
//  train_throttle
//
//  Created by Daniel BRAUN on 10/10/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface SimTrain : NSObject

@property (nonatomic,readonly) double speed;

- (void)setVolt:(double)volt forCantonNum:(int)numc;
- (void)setPwm:(double)duty dir:(int)d forCantonNum:(int)numc;

- (void) computeTrainsAfter:(uint32_t)ellapsed sinceStart:(uint32_t)ts;

- (double) bemfForCantonNum:(int)numc;

- (void) setTrain:(int)tidx sblk:(int)sblk posmm:(int)posmm;


- (NSString *) htmlSimuStateForTrain:(int)tidx;


@end


void ina_simu_tick(uint32_t notif_flags, uint32_t tick,  uint32_t dt);

NS_ASSUME_NONNULL_END
