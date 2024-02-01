//
//  TrainParamController.m
//  train_throttle
//
//  Created by Daniel Braun on 31/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#import "TrainParamController.h"
#import "conf_train.h"

@implementation TrainParamController

- (char) paramChar
{
    return 'T';
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return NUM_TRAINS;
}

@end
