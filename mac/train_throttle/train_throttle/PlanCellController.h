//
//  PlanCellController.h
//  train_throttle
//
//  Created by Daniel Braun on 27/09/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface PlanCellController : NSViewController

@property (nonatomic) NSInteger trainNum;
@property (nonatomic,readonly) NSString *trainName;
@property (nonatomic) NSInteger targetLSBLK;
@property (nonatomic) BOOL hasRule;
@property (nonatomic) NSUInteger delay;
- (void) resetEntry;

@end

NS_ASSUME_NONNULL_END
