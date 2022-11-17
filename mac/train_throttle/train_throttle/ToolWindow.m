//
//  ToolWindow.m
//  train_throttle
//
//  Created by Daniel Braun on 16/11/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import "ToolWindow.h"

@implementation ToolWindow


- (void) updateWindowMenu:(BOOL)visible
{
    NSString *s = [self title];
    if (!visible) s = [NSString stringWithFormat:@"(%@)", s];
    [NSApp changeWindowsItem:self title:s filename:false];
}
- (instancetype)initWithContentRect:(NSRect)contentRect styleMask:(NSWindowStyleMask)style backing:(NSBackingStoreType)backingStoreType defer:(BOOL)flag
{
    self = [super initWithContentRect:contentRect styleMask:style backing:backingStoreType defer:flag];
    if (self) {
        self.excludedFromWindowsMenu = NO;
        self.hidesOnDeactivate = YES;
        //self.visible = NO; "visible at launch" should be unchecked
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW,  0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^(){
            [self updateWindowMenu:NO];
        });
    }
    return self;
}

- (void) orderFront:(id)sender
{
    [super orderFront:sender];
    [self updateWindowMenu:YES];
}

- (void) makeKeyAndOrderFront:(id)sender
{
    [super makeKeyAndOrderFront:sender];
    [self updateWindowMenu:YES];
    
}

- (BOOL) isExcludedFromWindowsMenu
{
    return NO;
}

- (void) close
{
    //id m = [NSApp windowsMenu];
    [super close];
    [self updateWindowMenu:NO];
    id m2 = [NSApp windowsMenu];
    //[NSApp updateWindowsItem:self];
    //[NSApp changeWindowsItem:self title:@"plop" filename:false];
}
@end
