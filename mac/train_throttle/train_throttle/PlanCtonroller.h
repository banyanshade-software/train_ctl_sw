//
//  PlanCtonroller.h
//  train_throttle
//
//  Created by Daniel Braun on 27/09/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface PlanCtonroller : NSObject <NSTableViewDelegate,NSTableViewDataSource>

@property (nonatomic, weak) IBOutlet NSTableView *tableview;

- (IBAction) resetAll:(id)sender;
- (IBAction) startAllDelay:(id)sender;
- (IBAction) startAllNow:(id)sender;

@property (nonatomic) NSInteger startDelay;

@end

NS_ASSUME_NONNULL_END
