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

- (void) computeTrainsAfter:(NSTimeInterval)ellapsed sinceStart:(NSTimeInterval)ts;

- (double) bemfForCantonNum:(int)numc;


//- (int) simuCurCanton;
//- (int) simuNextCanton;

@end

NS_ASSUME_NONNULL_END
