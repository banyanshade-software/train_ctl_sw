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
+ (void) registerInstance:(ParamTableController *)s
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


- (void)awakeFromNib
{
    [super awakeFromNib];
    if (!_initDone) {
        [[self class]registerInstance:self];
        paramValues = [[NSMutableDictionary alloc]initWithCapacity:50];
        //[self initParams];
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

- (NSArray<NSString *> *) columnsParamIds
{
    NSAssert(_tableview, @"tableview not set");
    NSArray *cl = [_tableview tableColumns];
    NSAssert(([cl isKindOfClass:[NSArray class]]), @"should have an array");
    NSMutableArray *res= [NSMutableArray arrayWithCapacity:10];
    for (NSTableColumn *col in cl) {
        NSString *s = [col identifier];
        if (!s) continue;
        if ([s isEqual:@""]) continue;
        if ([s isEqual:@"num"]) continue;
        if ([s hasPrefix:@"Auto"]) continue;
        [res addObject:s];
    }
    return res;
}

- (NSArray<NSString *> *) getAllParamNames
{
    //NSInteger nc = _tableview.numberOfColumns;
    NSInteger nr = _tableview.numberOfRows;
    NSArray *parids = [self columnsParamIds];
    char pchar = [self paramChar];
    NSMutableArray *res = [NSMutableArray arrayWithCapacity:200];
    for (NSInteger r = 0; r < nr; r++) {
        for (NSString *fld in parids) {
            NSString *pnam = [NSString stringWithFormat:@"par_%c%d_%@",
                              pchar, (int)r, fld];
            [res addObject:pnam];
        }
    }
    return res;
    
}

#pragma mark -

- (void) setParam:(NSString *)pnam  parChar:(char)c value:(int32_t)v
{
    NSAssert(c==[self paramChar], @"bad controller");
    //NSInteger col = [_tableview columnWithIdentifier:field];
    [paramValues setObject:[NSNumber numberWithInt:v] forKey:pnam];
    [self setNeedReload];
}

- (void) setNeedReload
{
    // TODO: coales
    [_tableview reloadData];
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


#pragma mark -

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


- (BOOL)tableView:(NSTableView *)tableView shouldEditTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *cn = [tableColumn identifier];
    if ([cn isEqual:@"num"]) return NO;
    return YES;
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(nullable id)object forTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row;
{
    NSString *cn = [tableColumn identifier];
    if (![cn length]) return;
    if ([cn isEqual:@"num"]) return;
    NSString *paramid = [self idForRow:row col:cn];
    int v = [object intValue];
    AppDelegate *appdelegate = [[NSApplication sharedApplication] delegate];
    [appdelegate updateParameter:paramid value:v];
}

@end
