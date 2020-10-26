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

- (void)setVolt:(double)volt;
- (void)setPwm:(double)duty dir:(int)d;
- (double) bemfAfter:(NSTimeInterval)ellapsed sinceStart:(NSTimeInterval)ts;

@end

NS_ASSUME_NONNULL_END
