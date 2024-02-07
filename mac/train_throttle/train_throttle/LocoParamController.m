//
//  LocoParamController.m
//  train_throttle
//
//  Created by Daniel Braun on 31/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#import "LocoParamController.h"
#import "conf_locomotive.h"
#import "conf_locomotive.propag.h"
#import "train2loco.h"
#import "locomotives.h"

@implementation LocoParamController

- (char) paramChar
{
    return 'L';
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return NumKnownLoco;
}

- (NSInteger)paramNum
{
    return conf_lnum_locomotive;
}



- (nullable id)tableView:(NSTableView *)tableView objectValueForTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *cn = [tableColumn identifier];
    if ([cn isEqual:@"num"]) {
        if (row>=NumKnownLoco) return @"???";
        if (row<0) return @"???";
        const char *str = locomotiveNames[row];
        return [NSString stringWithCString:str encoding:NSUTF8StringEncoding];
    }
    return [super tableView:tableView objectValueForTableColumn:tableColumn row:row];
}
@end
