//
//  Reader.m
//  layoutexport
//
//  Created by Daniel BRAUN on 26/05/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import "Reader.h"
#import "dumped.h"

@implementation Reader

- (BOOL) readLayoutFile:(NSString *)path
{
    NSURL *url = [NSURL fileURLWithPath:path];
    NSData *d = [[NSData alloc]initWithContentsOfURL:url];
    NSUInteger len = [d length];
    //d = [d subdataWithRange:NSMakeRange(16, len-16)];
    NSError *err;
    id x = [NSKeyedUnarchiver unarchivedObjectOfClass:[NSObject class] fromData:d error:&err];
    NSLog(@"x:%@\n", x);
    
    id y = [NSUnarchiver unarchiveObjectWithData:d];
    NSLog(@"y:%@\n", y);
    
    //NSPropertyListFormat NSPropertyListBinaryFormat_v1_0 NSPropertyListOpenStepFormat
    NSPropertyListFormat f = NSPropertyListBinaryFormat_v1_0;
    id z = [NSPropertyListSerialization propertyListWithData:d options:kCFPropertyListImmutable format:&f error:&err];
    NSLog(@"z:%@\n", z);

    id a = [[NSAKDeserializer alloc]init];
    id az = [a deserializeData:d];
    return YES;
}
@end
