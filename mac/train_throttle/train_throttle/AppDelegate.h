//
//  AppDelegate.h
//  train_throttle
//
//  Created by Daniel BRAUN on 20/09/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <WebKit/WebKit.h>

@class SimTrain;
@class CTCManager;


@interface AppDelegate : NSObject <NSApplicationDelegate,
    CBCentralManagerDelegate,CBPeripheralDelegate,
    NSTableViewDelegate,NSTableViewDataSource,
    WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler>

@property (nonatomic,readonly) int curspeed;
@property (nonatomic) int dspeedT0;
@property (nonatomic) int dspeedT1;
@property (nonatomic) int dspeedT2;
@property (nonatomic) int dspeedT3;
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

@property (nonatomic, weak) IBOutlet NSSlider *sliderTarget1;
@property (nonatomic, weak) IBOutlet NSSlider *sliderTarget2;
@property (nonatomic, weak) IBOutlet NSSlider *sliderTarget3;

@property (nonatomic, weak) IBOutlet NSView *paramView1;
@property (nonatomic, weak) IBOutlet NSView *paramView2;
@property (nonatomic, weak) IBOutlet NSView *paramView3;
@property (nonatomic, weak) IBOutlet NSView *paramView4;
@property (nonatomic, weak) IBOutlet NSTableView *cantonTableView;
@property (nonatomic, weak) IBOutlet NSTableView *trainTableView;


@property (nonatomic, weak) IBOutlet CTCManager *ctcManager;

@property (nonatomic) double target_bemf;

@property (nonatomic) int numtrains;
@property (nonatomic) int numcantons;

@property (nonatomic) int oamCustomBrd;
@property (nonatomic) int oamCustomCmd;

@property (nonatomic) double T0_bemf_mv;
//@property (nonatomic) double bemfiir_centivolts;
//@property (nonatomic) double canton_0_centivolts;
//@property (nonatomic) double canton_0_centivon;
@property (nonatomic) int    C0_vidx;
@property (nonatomic) int    C0_pwm;
//@property (nonatomic) double canton_0_intensity;
//@property (nonatomic) double canton_0_ion;
//@property (nonatomic) double canton_0_ioff;


//@property (nonatomic) double T0_bemf_mv;
//@property (nonatomic) double canton_1_bemf_lp;
//@property (nonatomic) double canton_1_centivolts;
@property (nonatomic) int    C1_vidx;
@property (nonatomic) int    C1_pwm;
//@property (nonatomic) double canton_1_intensity;

@property (nonatomic) int T0_pose;

- (IBAction) changeParam:(id)sender;

- (IBAction) defaultInertia:(id)sender;
- (IBAction) zeroInertia:(id)sender;
- (IBAction) defaultPwmMinMax:(id)sender;

- (IBAction) startAuto:(id)sender;
//- (IBAction) stopAuto:(id)sender;
@property (nonatomic) int isAuto;
@property (nonatomic) int train0_auto_spd;
@property (nonatomic) int train0_auto_state;

@property (nonatomic) int autoNum;

@property (nonatomic) int ledProg;
- (IBAction) sendLed:(id)sender;

// turnout is identified by tag
- (IBAction) turnoutA:(id)sender;
- (IBAction) turnoutB:(id)sender;

- (IBAction) clearePose:(id)sender;

- (IBAction) startCalibration:(id)sender;
//@property (nonatomic,strong) NSMutableAttributedString *attribLog;

- (IBAction) connectSimu:(id)sender;

- (IBAction) triggerOscillo:(id)sender;
- (IBAction) recordMsg:(id)sender;

- (IBAction) startRecord:(id)sender;
- (IBAction) markRecord:(id)sender;
- (IBAction) stopRecord:(id)sender;
@property (nonatomic, strong) NSString *recordName;
@property (nonatomic, strong) NSURL *fileURL;
@property (nonatomic) int recordState;
@property (nonatomic,weak) IBOutlet NSTextView *logView;
@property (nonatomic) int plotNum;
- (IBAction) plotRecord:(id)sender;

// test mode window

@property (nonatomic) NSUInteger runMode;
@property (nonatomic) NSUInteger testCanton;
@property (nonatomic) NSUInteger testVoltIdx;
@property (nonatomic) NSInteger testPWM;

- (IBAction) startBLE:(id)sender;

- (IBAction) setTrainMode:(id)sender;

- (void) toggleTurnout:(int)tn;


@property (nonatomic, weak) IBOutlet NSWindow *trainParamWin;

@end

