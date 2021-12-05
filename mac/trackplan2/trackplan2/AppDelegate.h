//
//  AppDelegate.h
//  trackplan2
//
//  Created by Daniel BRAUN on 04/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class CTCManager;

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (nonatomic) int T0_to;
@property (nonatomic) int T0_from;

@property (nonatomic) int T1_to;
@property (nonatomic) int T1_from;

@property (nonatomic) int T2_to;
@property (nonatomic) int T2_from;

@property (nonatomic) int T3_to;
@property (nonatomic) int T3_from;

@property (nonatomic) double q_alpha;
@property (nonatomic) double q_gamma;
@property (nonatomic) double q_epsilon;

@property (nonatomic, weak) IBOutlet CTCManager *ctcManager;

- (IBAction) initQ:(id)sender;
- (IBAction) runQ:(id)sender;
@end

