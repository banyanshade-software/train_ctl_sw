//
//  ParamTableController.h
//  train_throttle
//
//  Created by Daniel Braun on 31/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface ParamTableController : NSObject <NSTableViewDelegate,NSTableViewDataSource>
@property (nonatomic, weak) IBOutlet NSTableView *tableview;


- (NSArray<NSString *> *) columnsParamIds;
- (NSArray<NSString *> *) getAllParamNames;
- (void) setParam:(NSString *)pnam  parChar:(char)c value:(int32_t)v;

- (void) setNeedReload;
- (void) clearParams;


// should be redefined by subclasses. returns 'T' for train, etc..
- (char) paramChar;
// should be redefined by subclasses.
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView;

+ (NSArray *) instances;

- (NSInteger)paramNum;

- (IBAction) commitChanges:(id)sender;

@end

NS_ASSUME_NONNULL_END
