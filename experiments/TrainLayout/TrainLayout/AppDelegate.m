//
//  AppDelegate.m
//  TrainLayout
//
//  Created by Daniel BRAUN on 07/11/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "AppDelegate.h"
#import "LayoutItem.h"
#import "TcoLayout.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    LayoutItem *l1 = [[LayoutItem alloc]initWithNum:1 name:@"cur" leftEnd:@[] rightEnd:@[@"C"] path:@"8-1\\"];
    LayoutItem *l2 = [[LayoutItem alloc]initWithNum:2 name:@"dte" leftEnd:@[] rightEnd:@[@"D"] path:@"7-"];
    LayoutItem *l3 = [[LayoutItem alloc]initWithNum:0 name:@"main" leftEnd:@[] rightEnd:@[@"C", @"D"] path:@"9-2/1|"];
    TcoLayout *tco = [[TcoLayout alloc]init];
    [tco layout:@[l1,l2,l3]];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


@end
