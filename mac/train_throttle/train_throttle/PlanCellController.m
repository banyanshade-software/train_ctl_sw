//
//  PlanCellController.m
//  train_throttle
//
//  Created by Daniel Braun on 27/09/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import "PlanCellController.h"

@interface PlanCellController ()

@end

@implementation PlanCellController
@synthesize trainNum = _trainNum;


- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    
    NSLog(@"..");
}
+ (NSSet *)keyPathsForValuesAffectingValueForKey:(NSString *)key
{
    NSSet *keyPaths = [super keyPathsForValuesAffectingValueForKey:key];
 
    if ([key isEqualToString:@"trainName"]) {
        NSArray *affectingKeys = @[@"trainNum"];
        keyPaths = [keyPaths setByAddingObjectsFromArray:affectingKeys];
    } /* else if ([key isEqualToString:@"trainNum"]) {
        NSArray *affectingKeys = @[@"trainName"];
        keyPaths = [keyPaths setByAddingObjectsFromArray:affectingKeys];
    }*/
    return keyPaths;
}
- (NSString *) trainName
{
    return [NSString stringWithFormat:@"T%d", (int) _trainNum];
}

- (void) resetEntry
{
    self.hasRule = NO;
}
@end
