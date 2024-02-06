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
    return NumKnownLoco-1;
}

- (NSInteger)paramNum
{
    return conf_lnum_locomotive;
}
@end
