//
//  ParamTableController.m
//  train_throttle
//
//  Created by Daniel Braun on 31/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#import "ParamTableController.h"
#import "AppDelegate.h"
#import "AppDelegateP.h"


@implementation ParamTableController {
    char _paramchar;
    NSMutableDictionary *paramValues;
    BOOL _initDone;
}

@synthesize tableview = _tableview;

static NSMutableArray *_instances = nil;
+ (NSArray *) instances
{
    return _instances;
}
+ (void) registerInstance:(NSMutableArray*)s
{
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        _instances = [[NSMutableArray alloc]initWithCapacity:5];
    });
    [_instances addObject:s];
}


- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    NSAssert(0, @"should be overriden");
    return 1;
}

 
- (void) initParams
{
    NSAssert(_tableview, @"tableview not set");
    NSInteger nc = _tableview.numberOfColumns;
    NSInteger nr = _tableview.numberOfRows;
}

- (void)awakeFromNib
{
    [super awakeFromNib];
    if (!_initDone) {
        [[self class]registerInstance:self];
        [self initParams];
        _initDone = YES;
    }
}
- (char) paramChar
{
    return 'T';
}

- (NSInteger)paramNum
{
    NSAssert(0, @"should be overriden");
    return 0;
}

- (NSArray<NSString *> *) columnsIds
{
    NSArray *cl = [_tableview tableColumns];
    NSAssert(([cl isKindOfClass:[NSArray class]]), @"should have an array");
    NSMutableArray *res= [NSMutableArray arrayWithCapacity:10];
    for (NSTableColumn *col in cl) {
        NSString *s = [col identifier];
        [res addObject:s];
    }
    return res;
}
#pragma mark -

- (NSString *) idForRow:(NSInteger)row col:(NSString *)col
{
    // par_T0_trainlen_left_cm
    if (!_paramchar) {
        _paramchar = [self paramChar];
    }
    return [NSString stringWithFormat:@"par_%c%d_%@", _paramchar, (int)row, col];
}

- (nullable id)tableView:(NSTableView *)tableView objectValueForTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *cn = [tableColumn identifier];
    if (![cn length]) return @"-";
    if ([cn isEqual:@"num"]) return @(row);
    NSString *paramid = [self idForRow:row col:cn];
    id value = [paramValues objectForKey:paramid];
    if (!value) return @"...";
    return value;
}

@end
