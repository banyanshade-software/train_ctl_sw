//
//  PlanCellView.h
//  train_throttle
//
//  Created by Daniel Braun on 27/09/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class PlanCellController;

@interface PlanCellView : NSTableCellView

@property (nonatomic, strong) IBOutlet PlanCellController *controller;

@end

NS_ASSUME_NONNULL_END
