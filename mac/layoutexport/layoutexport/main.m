//
//  main.m
//  layoutexport
//
//  Created by Daniel BRAUN on 26/05/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Reader.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"Hello, World!");
        Reader *reader = [[Reader alloc]init];
        BOOL ok = [reader readLayoutFile:@"/Users/danielbraun/Documents/test.layout"];
    }
    return 0;
}
