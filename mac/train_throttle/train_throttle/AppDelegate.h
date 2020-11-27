//
//  AppDelegate.h
//  train_throttle
//
//  Created by Daniel BRAUN on 20/09/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreBluetooth/CoreBluetooth.h>

@class SimTrain;

@interface AppDelegate : NSObject <NSApplicationDelegate,
    CBCentralManagerDelegate,CBPeripheralDelegate>

@property (nonatomic,readonly) int curspeed;
@property (nonatomic) int targetspeed;
@property (nonatomic) double train_bemf;
@property (nonatomic, readonly) int transmit;
@property (nonatomic, readonly) int linkok;
// affect only display :
@property (nonatomic, readwrite) int polarity;
@property (nonatomic, readwrite) int shunting;
@property (nonatomic, readonly) int minslider;
@property (nonatomic, readonly) int maxslider;

@property (nonatomic, readonly)  SimTrain *simTrain0;

- (IBAction) stopAll:(id)sender;
- (IBAction) goZero:(id)sender;

@property (nonatomic, weak) IBOutlet NSSlider *sliderTarget;
@property (nonatomic, weak) IBOutlet NSSlider *sliderCur;

@property (nonatomic, weak) IBOutlet NSView *paramView;

@property (nonatomic) double target_bemf;

@property (nonatomic) int numtrains;
@property (nonatomic) int numcantons;


@property (nonatomic) double canton_0_bemfcentivolt;
@property (nonatomic) double bemfiir_centivolts;
@property (nonatomic) double canton_0_centivolts;
@property (nonatomic) double canton_0_centivon;
@property (nonatomic) int    canton_0_pwm;
//@property (nonatomic) double canton_0_intensity;
@property (nonatomic) double canton_0_ion;
@property (nonatomic) double canton_0_ioff;


@property (nonatomic) double canton_1_bemfcentivolt;
//@property (nonatomic) double canton_1_bemf_lp;
@property (nonatomic) double canton_1_centivolts;
@property (nonatomic) double canton_1_centivon;
@property (nonatomic) int    canton_1_pwm;
@property (nonatomic) double canton_1_intensity;

@property (nonatomic) int train0_pose;

- (IBAction) changeParam:(id)sender;

- (IBAction) defaultInertia:(id)sender;
- (IBAction) zeroInertia:(id)sender;
- (IBAction) defaultPwmMinMax:(id)sender;

- (IBAction) startAuto:(id)sender;
- (IBAction) stopAuto:(id)sender;
@property (nonatomic) int isAuto;
@property (nonatomic) int train0_auto_spd;
@property (nonatomic) int train0_auto_state;


// turnout is identified by tag
- (IBAction) turnoutA:(id)sender;
- (IBAction) turnoutB:(id)sender;
- (IBAction) turnoutW:(id)sender;

- (IBAction) clearePose:(id)sender;

- (IBAction) startCalibration:(id)sender;
//@property (nonatomic,strong) NSMutableAttributedString *attribLog;

- (IBAction) connectSimu:(id)sender;



- (IBAction) startRecord:(id)sender;
- (IBAction) markRecord:(id)sender;
- (IBAction) stopRecord:(id)sender;
@property (nonatomic, strong) NSString *recordName;
@property (nonatomic, strong) NSURL *fileURL;
@property (nonatomic) int recordState;
@property (nonatomic,weak) IBOutlet NSTextView *logView;
@property (nonatomic) int plotNum;
- (IBAction) plotRecord:(id)sender;



- (IBAction) startBLE:(id)sender;

@end

