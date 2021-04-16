//
//  AppDelegate.m
//  train_throttle
//
//  Created by Daniel BRAUN on 20/09/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "AppDelegate.h"

#import "train_simu.h"
#import "trainctl_iface.h"
#import "SimTrain.h"
#include "statval.h"
#include "txrxcmd.h"

#include "StringExtension.h"



#define HIGHLEVEL_SIMU_CNX 0

/*
 * response format :
 * |s'R'rvv..|
 * notif format
 * |s'N'SNCvv..|
 */
#define MAX_DATA_LEN 512
typedef struct {
    uint8_t state;
    uint8_t escape;
    uint8_t seqnum;
    uint8_t notif;
    uint8_t retcode;
    uint8_t sel;
    uint8_t num;
    uint8_t cmd;
    uint16_t pidx;
    union { // ensure alignment
        uint8_t param[MAX_DATA_LEN];
        uint32_t param32[MAX_DATA_LEN/4];
    };
} frame_state_t;

// link ok values
#define LINK_DISC      0
#define LINK_CNX       1   // connected to tty
#define LINK_FRM       2   // frame mode set
#define LINK_OK        3   // fully ok
#define LINK_SIMUHI    4
#define LINK_SIMULOW   5

typedef void (^respblk_t)(void);

@interface AppDelegate () {
    int retry;
    NSFileHandle *usb;
    // frame decode
    frame_state_t frm;
    // seqnum
    int fsn;
    respblk_t resblk[256];
    // coalesce throttle command
    NSTimeInterval lastThrottle;
    NSTimer *timSendThrottle;
    int nparam;
    int nparamresp;

    // simu
    NSTimer *simuTimer;
    NSTimeInterval lastSimu;
    NSTimeInterval t0;
    SimTrain *simTrain0;
    
    // record
    NSFileHandle *recordFile;
    NSMutableDictionary *recordItems;
    NSMutableDictionary *cantons_value;
    // plot
    uint32_t graph_t0;
    NSTask *gnuplotTask;
    NSPipe *gnuplotStdin;
    NSPipe *gnuplotStdout;
    NSFileHandle *gnuplotStdinFh;
    NSFileHandle *gnuplotStdoutFh;
    
    
    CBCentralManager *cbcentral;
    CBPeripheral *trainctlBle;
    NSTimer *connectTimeout;
    BOOL connected;
}

@property (weak) IBOutlet NSWindow *window;
@property (nonatomic, readwrite) int curspeed;
@property (nonatomic, readwrite) int transmit;
@property (nonatomic, readwrite) int linkok;
@property (nonatomic, readwrite)  SimTrain *simTrain0;

@end

@implementation AppDelegate

@synthesize curspeed = _curspeed;
@synthesize targetspeed = _targetspeed;
@synthesize polarity = _polarity;
@synthesize simTrain0 = _simTrain0;
@synthesize recordState = _recordState;


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    signal(SIGPIPE, SIG_IGN);

    self.polarity = 1;
    self.shunting = 0;
    self.transmit = 0;
    self.linkok = 0;
    [self forAllParamsDo:^(NSControl *c){
        c.enabled = NO;
        self->nparam++;
    }];
    self.numtrains = 1;
    self.numcantons = 1+1;
    // for deebug
    //[self getParams]; //XXX XXX
    //[self startBLE];
    [self openUsb];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void) setShunting:(int)s
{
    if (s == _shunting) return;
    _shunting = s;
    
    int m = _shunting ? 30 : 100;
    self.sliderCur.minValue = -m;
    self.sliderCur.maxValue = m;
    [self willChangeValueForKey:@"minslider"];
    [self willChangeValueForKey:@"maxslider"];
    self.sliderTarget.minValue = -m;
    self.sliderTarget.maxValue = m;
    [self didChangeValueForKey:@"minslider"];
    [self didChangeValueForKey:@"maxslider"];
    self.sliderTarget.numberOfTickMarks= _shunting ? 3 : 21;
    if (_shunting) {
        if (_targetspeed<-m) self.targetspeed = -m;
        if (_targetspeed>m) self.targetspeed = m;
    }
}

- (int) minslider
{
    return  self.sliderTarget.minValue;
}
- (int) maxslider
{
    return self.sliderTarget.maxValue;
}
- (void) setPolarity:(int)p
{
    if (_polarity == p) return;
    _polarity = p;
    self.curspeed = - _curspeed;
    self.targetspeed = - _targetspeed;
}
- (void) goZero:(id)sender
{
    [self setTargetspeed:0];
}
- (void) setTargetspeed:(int)v
{
    if (_linkok < LINK_OK) return;
    NSLog(@"targetspeed %d", v);
    if (_targetspeed==v) return;
    _targetspeed = v;
    // coalesce to avoid too many command
    NSTimeInterval t = [NSDate timeIntervalSinceReferenceDate];
    if ([timSendThrottle isValid]) {
        NSLog(@"targetspeed timer running\n");
    }
    if ((t-lastThrottle) > 0.2) {
        [self sendTargetSpeed];
    } else {
        NSLog(@"targetspeed coalesce");
        timSendThrottle = [NSTimer timerWithTimeInterval:0.3 target:self selector:@selector(sendTargetSpeedFromTimer) userInfo:nil repeats:NO];
        [[NSRunLoop mainRunLoop]addTimer:timSendThrottle forMode:NSDefaultRunLoopMode];
    }
}
- (void) sendTargetSpeedFromTimer
{
    NSLog(@"targetspeed sendTargetSpeedFromTimer");
    [timSendThrottle invalidate];
    [self sendTargetSpeed];
}
- (void) sendTargetSpeed
{
    if (_linkok < LINK_OK) return;
    
    NSLog(@"targetspeed sendFrame");
    lastThrottle = [NSDate timeIntervalSinceReferenceDate];
    uint8_t spdfrm[] = "|xT\0V__|.........";
    int l = 2+4+2;
    uint16_t v16 = (uint16_t) (_polarity * _targetspeed);
    if (_linkok == LINK_SIMUHI) {
        train_set_target_speed(0, v16);
        return;
    }
    // assume same endienness (little)
    memcpy(spdfrm+5, &v16, 2);
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:nil];
}

- (IBAction) stopAll:(id)sender
{
    if ([timSendThrottle isValid]) {
        NSLog(@"...");
    }
    [timSendThrottle invalidate];
    timSendThrottle = nil;
    if (_linkok == LINK_SIMUHI) {
        train_stop_all();
    } else {
        uint8_t spdfrm[] = "|zG\0S|....";
        int l = 2+4+0;
        [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
            self.curspeed = 0;
        }];
    }
    [self willChangeValueForKey:@"targetspeed"];
    _targetspeed = 0;
    [self didChangeValueForKey:@"targetspeed"];
}


- (IBAction) startAuto:(id)sender
{
    uint8_t spdfrm[] = "|zT\0A|....";
    int l = 2+4+0;
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
        self.isAuto = 1;
    }];

}
- (IBAction) stopAuto:(id)sender
{
    uint8_t spdfrm[] = "|zT\0a|....";
       int l = 2+4+0;
       [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
           self.isAuto = 0;
       }];
}

- (IBAction) turnoutA:(id)sender
{
    uint8_t spdfrm[] = "|zA\0S|....";
    NSInteger t = [sender tag];
    spdfrm[3] = (uint8_t)t;
    int l = 2+4+0;
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
        NSLog(@"turnout done");
    }];
}

- (IBAction) turnoutB:(id)sender
{
    uint8_t spdfrm[] = "|zA\0s|....";
    NSInteger t = [sender tag];
    spdfrm[3] = (uint8_t)t;
    int l = 2+4+0;
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
        NSLog(@"turnout done");
    }];
}


- (IBAction) turnoutW:(id)sender
{
    uint8_t spdfrm[] = "|zA\0W|....";
    NSInteger t = [sender tag];
    spdfrm[3] = (uint8_t)t;
    int l = 2+4+0;
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
        NSLog(@"turnoutW done");
    }];
}

- (IBAction)clearePose:(id)sender
{
    uint8_t spdfrm[] = "|zT\0z|....";
    //NSInteger t = [sender tag];
    //spdfrm[3] = (uint8_t)t;
    int l = 2+4+0;
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
        NSLog(@"clearePose done");
    }];
}

#pragma mark -
- (void) addLog:(NSString *)msg important:(BOOL)b error:(BOOL)e
{
    static NSDictionary *da;
    static NSDictionary *db;
    static NSDictionary *de;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        da = @{ };
        db = @{ NSStrokeWidthAttributeName: @-3.0};
        de = @{ NSStrokeWidthAttributeName: @-3.0,
                NSForegroundColorAttributeName: [NSColor redColor] };
    });
    NSDictionary *attr = da;
    if (e) attr=de;
    else if (b) attr=db;
    NSAttributedString *m = [[NSAttributedString alloc]initWithString:msg attributes:attr];
    // we are on main thread here
    [[_logView textStorage]appendAttributedString:m];
    [_logView scrollRangeToVisible:NSMakeRange([[_logView string] length], 0)];

}
                             
#pragma mark -

- (IBAction) connectSimu:(id)sender
{
    if (_linkok != LINK_DISC) return;
    theDelegate = self;
    railconfig_setup_default();
#if HIGHLEVEL_SIMU_CNX
    self.linkok = LINK_SIMU;
#else
    self.linkok = LINK_SIMULOW;
    [self performSelector:@selector(getParams) withObject:nil afterDelay:0.3];
#endif
    [self startSimu];
}

#pragma mark -

- (void) performForChildOf:(NSView *)parent having:(BOOL (^)(NSView *))cond action:(void(^)(NSView *))action
{
    NSArray *sa = [parent subviews];
    for (NSView *sv in sa) {
        [self performForChildOf:sv having:cond action:action];
        if (cond(sv)) action(sv);
    }
}

- (void) forAllParamsDo:(void(^)(NSControl *))action
{
    [self performForChildOf:self.paramView having:^(NSView *v) {
        NSString *s = v.identifier;
        if (![s isKindOfClass:[NSString class]]) return NO;
        if (![s hasPrefix:@"par_"]) return NO;
        NSLog(@"cond/param '%@'\n", s);
        return YES;
    } action:(void(^)(NSView *))action];
}

- (void) forParams:(NSArray *)parlist do:(void(^)(NSControl *))action
{
    NSSet *parset = [NSSet setWithArray:parlist];
    [self performForChildOf:self.paramView having:^(NSView *v) {
        NSString *s = v.identifier;
        if (![s isKindOfClass:[NSString class]]) return NO;
        if (![s hasPrefix:@"par_"]) return NO;
        if ([parset containsObject:s]) {
            NSLog(@"cond/param/set '%@'\n", s);
            return YES;
        }
        return NO;
    } action:(void(^)(NSView *))action];
}

- (IBAction) changeParam:(id)sender
{
    if (self.linkok < LINK_OK) return;
    
    NSTextField *c = (NSTextField *)sender;
    id x = c.identifier;
    int32_t v;
    if ([c isKindOfClass:[NSPopUpButton class]]) {
        NSPopUpButton *pb = (NSPopUpButton *)c;
        v = (int32_t)[pb selectedTag];
    } else {
        v = c.intValue;
    }
    NSLog(@"changeParam %@ -> %d", x, (int)v);
    
    uint8_t chgfrm[80] = "|xT\0Pvvvv......";
    NSArray *pa = [self splitParamName:x];
    NSString *psel = [pa objectAtIndex:1];
    NSString *pn = [pa objectAtIndex:2];
    if ([psel length] != 2) {
        return;
    }
    const char *cpsel = [psel cStringUsingEncoding:NSUTF8StringEncoding];
    const char *cpn = [pn cStringUsingEncoding:NSUTF8StringEncoding];
    NSUInteger nl = strlen(cpn);
    if (nl+4+1+1+4>=sizeof(chgfrm)) {
        return;
    }
    chgfrm[2]=cpsel[0];
    uint8_t n = cpsel[1];
    if (n>='0') n -= '0';
    chgfrm[3] = n;
    chgfrm[4] = 'P';
    memcpy(chgfrm+5, &v, sizeof(int32_t));
    memcpy(chgfrm+5+4, cpn, nl+1);
    chgfrm[5+nl+4+1] = '|';
    [self sendFrame:chgfrm len:(int)(5+nl+4+2) blen:sizeof(chgfrm) then:^{
        NSLog(@"changed rc=%d", self->frm.retcode);
    }];
    
}

- (NSArray *) splitParamName:(NSString *)s
{
    NSArray *pa = [s componentsSeparatedByString:@"_"];
    NSUInteger len = [pa count];
    NSArray *pn = [pa subarrayWithRange:NSMakeRange(2, len-2)];
    NSString *s2 = [pn componentsJoinedByString:@"_"];
    return @[ [pa objectAtIndex:0], [pa objectAtIndex:1], s2 ];
}


- (void) getParams
{
    nparamresp = 0;
    static uint8_t gpfrm[80] = "|xT\0p......";
    int __block n = 0;
    [self forAllParamsDo:^(NSControl *c){
        n++;
        /*if (0 && (n % 7)) {
            usleep(500*1000); // XXX to be fixed
        }*/
        NSString *s = c.identifier;
        //if (![s isKindOfClass:[NSString class]]) return NO;
        //if (![s hasPrefix:@"par_"]) return NO;
        NSArray *pa = [self splitParamName:s];
        if ([pa count] != 3) {
            return;
        }
        NSString *psel = [pa objectAtIndex:1];
        NSString *pn = [pa objectAtIndex:2];
        if ([psel length] != 2) {
            return;
        }
        const char *cpsel = [psel cStringUsingEncoding:NSUTF8StringEncoding];
        const char *cpn = [pn cStringUsingEncoding:NSUTF8StringEncoding];
        NSUInteger nl = strlen(cpn);
        if (nl+4+1+1>=sizeof(gpfrm)) {
            return;
        }
        gpfrm[2]=cpsel[0];
        uint8_t n = cpsel[1];
        if (n>='0') n -= '0';
        gpfrm[3] = n;
        gpfrm[4] = 'p';
        memcpy(gpfrm+5, cpn, nl+1);
        gpfrm[5+nl+1] = '|';
        NSLog(@"get param %c%c '%s'\n",  cpsel[0], cpsel[1], cpn);
    
        if ((1) & (0==(n%10))) {
            sleep(1);
        }
        [self sendFrame:gpfrm len:(int)(5+nl+2) blen:sizeof(gpfrm) then:^{
            // handle response
            self->nparamresp++;
            if (self->nparamresp==self->nparam) {
                if (self.linkok<LINK_OK) self.linkok = LINK_OK;
            }
            if (self->frm.retcode) {
                NSLog(@"error getting param rc=%d", self->frm.retcode);
                return;
            }
            if (self->frm.pidx != 4*sizeof(int32_t)) {
                NSLog(@"bad len %d", self->frm.pidx);
                if (self->nparamresp==self->nparam) self.linkok = LINK_OK;
                return ;
            }
            NSAssert(self->frm.pidx == 4*sizeof(int32_t), @"wrong resp len");
            NSLog(@"param val");
            int32_t val, min, max, def;
            memcpy(&val, self->frm.param + 0*sizeof(int32_t), sizeof(int32_t));
            memcpy(&def, self->frm.param + 1*sizeof(int32_t), sizeof(int32_t));
            memcpy(&min, self->frm.param + 2*sizeof(int32_t), sizeof(int32_t));
            memcpy(&max, self->frm.param + 3*sizeof(int32_t), sizeof(int32_t));
            [self setParameter:c value:val def:def min:min max:max enable:YES];
            NSLog(@"val %d def %d min %d max %d\n", val, def ,min, max);
        }];
        
        //NSLog(@"hop");
    }];
}

- (void) setParameter:(NSControl *)c value:(int)v def:(int)def min:(int)min max:(int)max enable:(BOOL)ena
{
    if ((0)) {
    } else if ([[c identifier] isEqualToString:@"par_G0_numtrains"]) {
        self.numtrains = v;
    } else if ([[c identifier] isEqualToString:@"par_G0_numcantons"]) {
        self.numcantons = v;
    }
    if ([c respondsToSelector:@selector(selectItemWithTag:)]) {
        [(id)c selectItemWithTag:v];
    } else if ([c respondsToSelector:@selector(setIntValue:)]) {
        [c setIntValue:v];
        /*
    } else if ([c respondsToSelector:@selector(setValue:)]) {
        [c setValue:v];
    */
    } else {
        NSLog(@"cnt set value");
    }
    NSNumberFormatter *nf = c.cell.formatter;
    if (nf) {
        [nf setMaximum:@(max)];
        [nf setMinimum:@(min)];
    }
    
    if ([c respondsToSelector:@selector(setMinValue:)]) {
        [(id)c setMinValue:min];
    }
    if ([c respondsToSelector:@selector(setMaxValue:)]) {
        [(id)c setMaxValue:min];
    }
    
    if (ena) c.enabled = YES;
}


- (void) resetToDefault:(NSArray *)parlist
{
    static uint8_t gpfrm[80] = "|xT\0p......";
    [self forParams:parlist do:^(NSControl *c){
        NSString *s = c.identifier;
        //if (![s isKindOfClass:[NSString class]]) return NO;
        //if (![s hasPrefix:@"par_"]) return NO;
        NSArray *pa = [self splitParamName:s];
        if ([pa count] != 3) {
            return;
        }
        NSString *psel = [pa objectAtIndex:1];
        NSString *pn = [pa objectAtIndex:2];
        if ([psel length] != 2) {
            return;
        }
        const char *cpsel = [psel cStringUsingEncoding:NSUTF8StringEncoding];
        const char *cpn = [pn cStringUsingEncoding:NSUTF8StringEncoding];
        NSUInteger nl = strlen(cpn);
        if (nl+4+1+1>=sizeof(gpfrm)) {
            return;
        }
        gpfrm[2]=cpsel[0];
        uint8_t n = cpsel[1];
        if (n>='0') n -= '0';
        gpfrm[3] = n;
        gpfrm[4] = 'p';
        memcpy(gpfrm+5, cpn, nl+1);
        gpfrm[5+nl+1] = '|';
        NSLog(@"get param %c%c '%s'\n",  cpsel[0], cpsel[1], cpn);
        [self sendFrame:gpfrm len:(int)(5+nl+2) blen:sizeof(gpfrm) then:^{
            // handle response
            if (self->frm.retcode) {
                NSLog(@"error getting param rc=%d", self->frm.retcode);
                return;
            }
            if (self->frm.pidx != 4*sizeof(int32_t)) {
                NSLog(@"bad len %d", self->frm.pidx);
                if (self->nparamresp==self->nparam) self.linkok = LINK_OK;
                return ;
            }
            NSAssert(self->frm.pidx == 4*sizeof(int32_t), @"wrong resp len");
            NSLog(@"reset to default val");
            int32_t val, min, max, def;
            memcpy(&val, self->frm.param + 0*sizeof(int32_t), sizeof(int32_t));
            memcpy(&def, self->frm.param + 1*sizeof(int32_t), sizeof(int32_t));
            memcpy(&min, self->frm.param + 2*sizeof(int32_t), sizeof(int32_t));
            memcpy(&max, self->frm.param + 3*sizeof(int32_t), sizeof(int32_t));
            [self setParameter:c value:def def:def min:min max:max enable:YES];
            NSLog(@"val %d def %d min %d max %d\n", val, def ,min, max);
            [self changeParam:c];
        }];
    }];
}


- (void) resetToZero:(NSArray *)parlist
{
    static uint8_t gpfrm[80] = "|xT\0p......";
    [self forParams:parlist do:^(NSControl *c){
        NSString *s = c.identifier;
        //if (![s isKindOfClass:[NSString class]]) return NO;
        //if (![s hasPrefix:@"par_"]) return NO;
        NSArray *pa = [self splitParamName:s];
        if ([pa count] != 3) {
            return;
        }
        NSString *psel = [pa objectAtIndex:1];
        NSString *pn = [pa objectAtIndex:2];
        if ([psel length] != 2) {
            return;
        }
        const char *cpsel = [psel cStringUsingEncoding:NSUTF8StringEncoding];
        const char *cpn = [pn cStringUsingEncoding:NSUTF8StringEncoding];
        NSUInteger nl = strlen(cpn);
        if (nl+4+1+1>=sizeof(gpfrm)) {
            return;
        }
        gpfrm[2]=cpsel[0];
        uint8_t n = cpsel[1];
        if (n>='0') n -= '0';
        gpfrm[3] = n;
        gpfrm[4] = 'p';
        memcpy(gpfrm+5, cpn, nl+1);
        gpfrm[5+nl+1] = '|';
        NSLog(@"get param %c%c '%s'\n",  cpsel[0], cpsel[1], cpn);
        [self sendFrame:gpfrm len:(int)(5+nl+2) blen:sizeof(gpfrm) then:^{
            // handle response
            if (self->frm.retcode) {
                NSLog(@"error getting param rc=%d", self->frm.retcode);
                return;
            }
            if (self->frm.pidx != 4*sizeof(int32_t)) {
                NSLog(@"bad len %d", self->frm.pidx);
                if (self->nparamresp==self->nparam) self.linkok = LINK_OK;
                return ;
            }
            NSAssert(self->frm.pidx == 4*sizeof(int32_t), @"wrong resp len");
            NSLog(@"resetToZero val");
            int32_t val, min, max, def;
            memcpy(&val, self->frm.param + 0*sizeof(int32_t), sizeof(int32_t));
            memcpy(&def, self->frm.param + 1*sizeof(int32_t), sizeof(int32_t));
            memcpy(&min, self->frm.param + 2*sizeof(int32_t), sizeof(int32_t));
            memcpy(&max, self->frm.param + 3*sizeof(int32_t), sizeof(int32_t));
            [self setParameter:c value:0 def:def min:min max:max enable:YES];
            NSLog(@"val %d def %d min %d max %d\n", val, def ,min, max);
            [self changeParam:c];
        }];
    }];
}
- (IBAction) defaultInertia:(id)sender
{
    [self resetToDefault:@[@"par_T0_acc", @"par_T0_dec"]];
}
- (IBAction) zeroInertia:(id)sender
{
    [self resetToZero:@[@"par_T0_acc", @"par_T0_dec"]];
}
- (IBAction) defaultPwmMinMax:(id)sender
{
    [self resetToDefault:@[@"par_T0_minpwm", @"par_T0_maxpwm"]];
}

- (IBAction) startCalibration:(id)sender
{
    uint8_t spdfrm[] = "|xG\0K|.........";
    int l = 2+4+0;
    uint16_t v16 = (uint16_t) (_polarity * _targetspeed);
    if (_linkok == LINK_SIMUHI) {
        return;
    }
    // assume same endienness (little)
    memcpy(spdfrm+5, &v16, 2);
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:nil];
}

#pragma mark -

#define USBTTY1 "/dev/cu.usbmodem6D94487754571"
#define USBTTY2 "/dev/cu.usbmodem376A356634381"

- (void) openUsb
{
    if ((_linkok == LINK_SIMUHI) || (_linkok == LINK_SIMULOW)) return;
    retry++;
    int fd = open((retry %2) ? USBTTY2 : USBTTY1, O_RDWR|O_NOCTTY);
    if (fd<0) {
        self.linkok = NO;
        usb = nil;
        if (errno != ENOENT) {
            perror("open:");
        }
        [self performSelector:@selector(openUsb) withObject:nil afterDelay:1.0];
        return;
    }
#define START_CMD "version\n" // nmode frame\r"
    write(fd, START_CMD, strlen(START_CMD));
    usb = [[NSFileHandle alloc]initWithFileDescriptor:fd closeOnDealloc:YES];
    //[usb readInBackgroundAndNotify];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(readUsbTty:) name:NSFileHandleDataAvailableNotification  object:usb];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(exceptionUsbTty:) name:NSFileHandleOperationException  object:usb];
    [usb waitForDataInBackgroundAndNotify];
    NSLog(@"connected");
    [self performSelector:@selector(goFrame) withObject:nil afterDelay:0.5];
}

- (void) goFrame
{
    NSLog(@"go frame\n");
    //nparam = 0;
    nparamresp = 0;
    self.transmit = 0;
#define FRM_CMD "mode frame\n"
    int fd = [usb fileDescriptor];
    write(fd, FRM_CMD, strlen(FRM_CMD));
    self.linkok = LINK_FRM;
    //self.linkok = LINK_OK; // XXX
    [self performSelector:@selector(getParams) withObject:nil afterDelay:0.2];
    //[self performSelector:@selector(getParams) withObject:nil afterDelay:2.2];
}


#pragma mark -


- (void) disconnect
{
    if (!usb) return;
    int fd = [usb fileDescriptor];
    close(fd);
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSFileHandleDataAvailableNotification object:usb];
    usb = nil;
    self.linkok = 0;
    self.transmit = 0;
    [self performSelector:@selector(openUsb) withObject:nil afterDelay:1.0];
}
- (void)readUsbTty:(NSNotification*)not
{
    NSData *d;
    @try {
        d = [usb availableData];
    } @catch (NSException *exception) {
        NSLog(@"exception %@", exception);
        d = nil;
        [self disconnect];
    } @finally {
    }
    if (!usb) {
        return;
    }
    /*NSLog(@"read:  %@\n", d);
    NSString *s = [[NSString alloc]initWithData:d encoding:NSUTF8StringEncoding];
    NSLog(@"str: %@\n", s); */
    [self processFrames:d];
    [usb waitForDataInBackgroundAndNotify];
}

- (void) exceptionUsbTty:(NSNotification *)not
{
    NSLog(@"exceptionUsbTty:");
}



#define FRAME_DELIM '|'
#define FRAME_ESC   '\\'

static int _frm_escape(uint8_t *buf, int len, int maxlen)
{
    int ne = 0;
    for (int i=0; i<len; i++) {
        if ((FRAME_ESC==buf[i]) || (FRAME_DELIM==buf[i])) {
            ne++;
        }
    }
    int nl = ne+len;
    if (nl>maxlen) return -1;
    for (int i=len-1+ne; i>=0; i--) {
        buf[i] = buf[i-ne];
        if ((FRAME_ESC==buf[i]) || (FRAME_DELIM==buf[i])) {
            buf[i-1]=FRAME_ESC;
            ne--;
            i--;
        }
    }
    return nl;
}

static int frm_escape(uint8_t *buf, int len, int maxlen)
{
    assert(buf[0]==FRAME_DELIM);
    assert(buf[len-1]==FRAME_DELIM);
    int l = _frm_escape(buf+1, len-2, maxlen-2);
    if (l<0) return l;
    buf[l+1]=FRAME_DELIM;
    return l+2;
}

/*
static int frm_unescape(uint8_t *buf, int len)
{
    int ne = 0;
    int ec = 0;
    for (int i=0; i<len; i++) {
        if (!ec) {
            if (('\\'==buf[i+ne]) || ('|'==buf[i+ne])) {
                ne++;
                ec = 1;
                continue;
            }
        }
        if (ne) {
            buf[i]=buf[i+ne];
        }
    }
    return len+ne;
}
*/
/*
 * response format :
 * |s'R'rvv..|
 * notif format
 * |s'N'SNCvv..|
 */

- (NSString *) dumpFrames:(NSData *)d
{
    NSMutableString *r = [[NSMutableString alloc]init];
    const uint8_t *dta = [d bytes];
    int l = (int) [d length];
    int e=0;
    int f=0;
    for (int i=0; i<l; i++) {
        uint8_t c = dta[i];
        if (isascii(c) && (c>32)) {
            [r appendFormat:@"%c ", c];
        } else {
            [r appendFormat:@"%2.2X ", c];
        }
        if ((FRAME_ESC == c) && !e) e=1;
        else {
            if ((FRAME_DELIM == c) && !e) {
                if (f) {
                    [r appendString:@"\n"];
                    f = 0;
                } else f=1;
            }
            e = 0;
        }
    }
    return r;
}

- (void) processFrames:(NSData *)d
{
    //static int trc=0;
#define TRC 0
    const uint8_t *dta;
    int l;
    l = (int) [d length];
    if ((TRC)) NSLog(@"recv %d bytes : %@\n", l, [self dumpFrames:d]);
    dta = [d bytes];
    for (int i=0; i<l; i++) {
        uint8_t c = dta[i];
        if ((TRC>1)) NSLog(@"st %d -- c %c %2.2X -- e=%d pidx=%d\n", frm.state, c, c, frm.escape, frm.pidx);
        
        if ((c == FRAME_DELIM) && !frm.escape) {
            if (0 == frm.state) {
                memset(&frm, 0, sizeof(frm));
                frm.state = 1;
            } else if (6 == frm.state) {
                // process frame
                if (frm.notif) {
                    [self frameNotif];
                } else {
                    [self frameResponse];
                }
                frm.state = 0;
                frm.escape = 0;
            } else {
                // short frame, ignore
                NSLog(@"short frame");
                frm.state = 1;
                frm.escape = 0;
            }
            continue;
        }
        if (c==FRAME_ESC && !frm.escape) {
            // state is >0 here
            //NSLog(@"escape");
            frm.escape = 1;
            continue;
        }
        switch (frm.state) {
            case 0:
                frm.escape = 0;
                continue;
            case 1:
                frm.seqnum = c;
                frm.state = 2;
                break;
            case 2:
                if ('R'==c) {
                    frm.state = 7;
                } else if ('N'==c) {
                    frm.notif = 1;
                    frm.state = 3;
                }
                break;
            case 3:
                frm.sel = c;
                frm.state = 4;
                break;
            case 4:
                frm.num = c;
                frm.state = 5;
                break;
            case 5:
                frm.cmd = c;
                frm.state = 6;
                break;
            case 6:
                if (frm.pidx >= MAX_DATA_LEN) {
                    NSLog(@"long frame");
                    frm.state = 0;
                    frm.escape = 0;
                    continue;
                }
                frm.param[frm.pidx] = c;
                frm.pidx ++;
                break;
            case 7:
                frm.retcode = c;
                frm.state = 6;
                break;
            default:
                NSAssert(0, @"bad state");
                break;
        }
        frm.escape = 0;
    }
}

- (void) frameNotif
{
    int16_t v,v2;
    int32_t v32;
    if ((0)) NSLog(@"notif / %c %d %c", frm.sel, frm.num, frm.cmd);
    if ('D' == frm.cmd) {
        int32_t v1, v2, v3;
        memcpy(&v1, frm.param, sizeof(int32_t));
        memcpy(&v2, frm.param+sizeof(int32_t), sizeof(int32_t));
        memcpy(&v3, frm.param+2*sizeof(int32_t), sizeof(int32_t));
        NSString *str = [NSString stringWithFormat:@"DBG %c%d : %s %d %d %d\r\n",
                         frm.sel, frm.num, frm.param+3*sizeof(int32_t), v1, v2, v3];
        NSLog(@"%@", str);
        [self addLog:str important:NO error:NO];
        return;
    }
    switch (frm.sel) {
        case 'G':
            switch (frm.cmd) {
                case 'E': {
                    int16_t c;
                    memcpy(&c, frm.param, 2);
                    NSLog(@"Error %d\n", c);
                    NSString *str = [NSString stringWithFormat:@"ERR %d : %s\n", c, frm.param+2];
                    [self addLog:str important:YES error:YES];
                    return;
                }
                case 'X': {
                    // stat frame
                    [self processStatFrame];
                    return;
                }
            }
            break;
        case 'C':
            switch (frm.cmd) {
                case 'b':
                case 'B':
                    if (frm.num == 0) {
                        int32_t v[4];
                        memcpy(&v, frm.param, 4*sizeof(int32_t));
                        //   unit 1/100 V
                        self.canton_0_bemfcentivolt =  BEMF_RAW ? v[0] : v[0]/100.0;
                        // IIR low pass on bemf
                       // self.canton_0_bemf_lp = _canton_0_bemf_lp*.97+0.03*_canton_0_bemfcentivolt;
                        if (frm.cmd == 'B') {
                            self.canton_0_centivon =   BEMF_RAW ? v[1] : v[1]/100.0;
                            self.canton_0_centivolts = v[2]/100.0;
                            self.canton_0_pwm =   v[3];
                        }
                    }
                    return;
            }
        case 'T': {
            struct spd_notif spd;
            switch (frm.cmd) {
                case 'V':
                    memcpy(&spd, frm.param, sizeof(spd));
                    //NSLog(@"train %d, v=%d\n", frm.num, spd.sv100);
                    if (0==frm.num || '0'==frm.num) { // XXX for test
                        self.curspeed = _polarity*spd.sv100;
                        self.target_bemf = (BEMF_RAW) ? spd.pid_target: spd.pid_target/100.0;
                        if ((1)) {
                            static int bmin = 0;
                            static int bmax = 0;
                            if (spd.bemf_centivolt > bmax) bmax = spd.bemf_centivolt;
                            if (spd.bemf_centivolt < bmin) bmin = spd.bemf_centivolt;
                            //NSLog(@"bemf %d [%d %d]", spd.bemf_centivolt, bmin, bmax);
                        }
                        self.train_bemf = spd.bemf_centivolt/100.0;
                       
                    }
                    return;
                    break;
                case 'i':
                    memcpy(&v32, frm.param, sizeof(int32_t));
                    self.train0_pose = v32;
                    return;
                    break;
                case 'A':
                    memcpy(&v, frm.param, 2);
                    memcpy(&v2, frm.param+2, 2);
                    self.train0_auto_state = v;
                    self.train0_auto_spd = v2;
                    return;
                    break;
            }
            break;
        
        }
    }
    NSLog(@"UNHANDLED notif / %c %d %c", frm.sel, frm.num, frm.cmd);
    NSString *str = [NSString stringWithFormat:@"UNHANDLED notif / %c %d %c", frm.sel, frm.num, frm.cmd];
    [self addLog:str important:YES error:NO];
}


- (void) processStatFrame
{
    NSAssert(frm.pidx>8, @"short stat frame");
    //if ((1)) return;
    
    cantons_value = [[NSMutableDictionary alloc]initWithCapacity:25];
    
    int nval = frm.pidx / 4;
    uint32_t *ptr = frm.param32;
    //uint32_t tick = *ptr++;
    nval--;
    int step = -1;
    for (;;nval--, step++) {
       
        int32_t v = *ptr++;
        off_t offset; int len;
        int trnidx=-1;
        int cntidx=-1;
        const char *name = NULL;
        if (step>=0) {
            int rc = get_val_info(step, &offset, &len, &trnidx, &cntidx, &name, self.numtrains, self.numcantons);
            if (rc) {
                // end
                if (nval>-1) {
                    NSLog(@"still has val after process");
                }
                goto done;
            }
        }
        if (nval<0) {
            NSLog(@"short stat frame");
            goto done;
        }
        //NSLog(@"stat : '%s'[%d,%d] = %d\n", name ? name : "_", trnidx,cntidx, v);
        NSString *key = nil;

        static NSMutableSet *unhandled_key = nil;
        if (!unhandled_key) unhandled_key = [[NSMutableSet alloc]initWithCapacity:20];
        if (name) {
            NSString *m = [NSString stringWithUTF8String:name];
            key = m;
            if (trnidx >= 0) {
                key = [NSString stringWithFormat:m, trnidx];
            } else if (cntidx >= 0) {
                key = [NSString stringWithFormat:m, cntidx];
            }
            NSNumber *nsv = @(v);
            if ([key hasPrefix:@"canton_"]) {
                [cantons_value setValue:nsv forKey:key];
            }
            if (![unhandled_key containsObject:key]) {
                //NSLog(@"---- %@\n", key);
                if ([key isEqualToString:@"canton_0_volts"]) {
                    NSLog(@"hop  canton_0_volts %@", nsv);
                }
                if ([key hasSubString:@"centi"]) {
                    nsv = @(v/100.0);
                }
                @try {
                    [self setValue:nsv forKey:key];
                } @catch (NSException *exception) {
                    NSLog(@"key %@ not ok: %@", key, exception);
                    [unhandled_key addObject:key];
                }
              
            }
        }
        if (_recordState == 1) {
            if (!recordItems) recordItems=[[NSMutableDictionary alloc]init];
            if (-1==step) {
                [recordFile writeData:[@"T_s, " dataUsingEncoding:NSUTF8StringEncoding]];
                graph_t0 = v;
            } else {
                if (key) [recordItems setObject:@(step+1) forKey:key];
                NSString *s = [NSString stringWithFormat:@"%@, ", key ? key : @"_"];
                [recordFile writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
            }
        } else if (_recordState) {
            NSString *s;
            if ([key hasSubString:@"centi"]) {
                    s = [NSString stringWithFormat:@"%f, ", v/100.0];
            } else if (step==-1) {
                        s = [NSString stringWithFormat:@"%f, ", (v-graph_t0)/1000.0];
            } else {
                    s = [NSString stringWithFormat:@"%d, ", v];
            }
            [recordFile writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
        }
    }
done:
    if (_recordState) {
        [recordFile writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];
        _recordState = 2;
    }
    [self.cantonTableView reloadData];
}


- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.numcantons;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSString *cn = [tableColumn identifier];
    if (![cn length]) return @"-";
    if ([cn isEqual:@"canton_num"]) return @(row);
    NSString *k = [NSString stringWithFormat:@"canton_%d_%@", (int)row, cn];
    NSNumber *n = [cantons_value objectForKey:k];
    if (cantons_value && !n) {
        NSLog(@"unknown %@",k);
    }
    return n;
}


#pragma mark -

- (void) frameResponse
{
    NSLog(@"response : SN=%d rc=%d plen=%d\n", frm.seqnum, frm.retcode, frm.pidx);
    self.transmit--;
    
    respblk_t b = resblk[frm.seqnum];
    resblk[frm.seqnum] = nil;
    if (!b) {
        NSLog(@"NO CALLBACK");
        [self stopAll:nil];
        NSAssert(b, @"no callback blck");
    }
    b();
    
}


- (void) sendFrame:(uint8_t *)frame len:(int)len blen:(int)blen then:(respblk_t)b
{
    if (_linkok<LINK_FRM) return;
    /*
    if (_transmit > 7) {
        NSLog(@"delay transmit");
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(100 * NSEC_PER_MSEC)), dispatch_get_main_queue(), ^{
            [self sendFrame:frame len:len blen:blen then:b];
        });
    }*/
    
    if (!b) b = ^{};
    int ssn = -1;
    for (int i=0; i<256; i++) {
        int s = (fsn+i)%256;
        //if ('z'==s) continue;
        if (resblk[s]) continue;
        ssn = s;
        break;
    }
    if (ssn == -1) {
        NSLog(@"no more sn");
        // [self disconnect];
        return;
    }
    fsn = ssn;
    NSLog(@"send frm SN=%d (%d)\n", ssn, len);
    frame[1]=(uint8_t)fsn;
    fsn = (fsn+1)%256;

    len = frm_escape(frame, len, blen);
    if (len<0) {
        NSLog(@"buffer too small for escape");
        NSAssert(0, @"buffer too small for escape");
        return;
    }
    
    resblk[ssn] = b;
    self.transmit++;
    // XXX SIMULOW
#if HIGHLEVEL_SIMU_CNX
#else
    if (self.linkok == LINK_SIMULOW) {
        frame_msg_t frresp;
        frresp.t = TXFRAME_TYPE_RESP;
        //void txrx_process_char(uint8_t c, uint8_t *respbuf, int *replen);
        for (int i=0; i<len; i++) {
            int rlen = FRM_MAX_LEN;
            txrx_process_char(frame[i], frresp.frm, &rlen);
            if (rlen>0) {
                NSData *d = [NSData dataWithBytes:frresp.frm length:rlen];
                if ((TRC)) NSLog(@"from simu  resp %d bytes : %@\n", rlen, [self dumpFrames:d]);
                [self performSelectorOnMainThread:@selector(processFrames:) withObject:d waitUntilDone:NO];
            }
        }
        return;
    }
#endif
    int fd = [usb fileDescriptor];
    write(fd, frame, len);
}


#pragma mark - simu

static AppDelegate *theDelegate = nil;

- (void) startSimu
{
    self.simTrain0 = [[SimTrain alloc]init];
    lastSimu = [NSDate timeIntervalSinceReferenceDate];
    t0 = lastSimu;
    simuTimer = [NSTimer timerWithTimeInterval:0.1 target:self selector:@selector(simuTimer) userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop]addTimer:simuTimer forMode:NSDefaultRunLoopMode];
    
}

static int task_auto_notif = 0;

void task_auto_start_auto(void)
{
    task_auto_notif |= AUTO1_NOTIF_CMD_START;
}

void task_auto_stop_auto(void)
{
    task_auto_notif |= AUTO1_NOTIF_CMD_STOP;
}
- (void) simuTimer
{
    static NSTimeInterval t0;
    static dispatch_once_t onceToken=0;
    dispatch_once(&onceToken, ^{
        t0 = [NSDate timeIntervalSinceReferenceDate];
    });
    NSTimeInterval t = [NSDate timeIntervalSinceReferenceDate];
    NSTimeInterval dt = t-lastSimu;
    lastSimu = t;
    uint32_t ticks = (uint32_t)(dt * 1000.0);
    double bemf = [_simTrain0 bemfAfter:ticks sinceStart:(t-t0)];
    // xxxx
    int bemfi = -(bemf/4.545) * 3.3 *4096;
    //NSLog(@"bemf %f -> %d\n", bemf, bemfi);
    
    for (int i =0; i<NUM_LOCAL_CANTONS; i++) {
        memset((void*)&(train_adc_buffer[i]), 0, sizeof(adc_buffer_t));
    }
    int n1 = [_simTrain0 simuCurCanton];
    if (n1>=0) {
        train_adc_buffer[n1].voffA=(bemfi>0) ? 0 : -bemfi;
        train_adc_buffer[n1].voffB=(bemfi>0) ? bemfi : 0;
        train_adc_buffer[n1].intOn=1500;
    }
    int n2 = [_simTrain0 simuNextCanton];
    if (n2>=0) {
        train_adc_buffer[n2].voffA=(bemfi>0) ? 0 : -bemfi;
        train_adc_buffer[n2].voffB=(bemfi>0) ? bemfi : 0;
        train_adc_buffer[n2].intOn=1500;
    }

    train_run_tick(NOTIF_NEW_ADC_1, (t-t0)*1000, ticks);
    task_auto_notif |= AUTO1_NOTIF_TICK;
    auto1_run(task_auto_notif, (t-t0)*1000);
    task_auto_notif = 0;
}

void train_simu_canton_volt(int numcanton, int voltidx, int vlt100)
{
    [theDelegate simuSetVoltCanton:numcanton voltidx:voltidx vlt100:vlt100];
}

- (void) simuSetVoltCanton:(int)numcanton voltidx:(int)voltidx vlt100:(int)vlt100
{
    double vlt = vlt100/100.0;
    if (numcanton == [_simTrain0 simuCurCanton]) {
        [_simTrain0 setVolt:vlt];
    }
    switch (numcanton) {
        case 0:
            self.canton_0_centivolts = vlt;
            [_simTrain0 setVolt:vlt];
            break;
        case 1:
            self.canton_1_centivolts = vlt;
            break;
        default:
            break;
    }
}
void train_simu_canton_set_pwm(int numcanton, int dir, int duty)
{
    [theDelegate simuSetPwmCanton:numcanton dir:dir duty:duty];
}

- (void) simuSetPwmCanton:(int)numcanton dir:(int)dir duty:(int)duty
{
    int sduty = duty*dir;
    if (numcanton == [_simTrain0 simuCurCanton]) {
        [_simTrain0 setPwm:duty dir:dir];
    }
    switch (numcanton) {
        case 0:
            self.canton_0_pwm = sduty;
            break;
        case 1:
            self.canton_1_pwm = sduty;
        default:
            break;
    }
}

// connection to trainctl simu can be either
// at char/frame level or at higher level

#if HIGHLEVEL_SIMU_CNX

void trainctl_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen)
{
    [theDelegate simuNotifSel:sel num:num cmd:cmd dta:dta dtalen:dtalen];
}
- (void) simuNotifSel:(uint8_t)sel num:(uint8_t)num cmd:(uint8_t)cmd dta:(uint8_t *)dta dtalen:(int)dtalen
{
    frm.sel = sel;
    frm.num = num;
    frm.cmd = cmd;
    NSAssert(dtalen<sizeof(frm.param), @"too long data");
    memcpy(frm.param, dta, dtalen);
    [self frameNotif];
}

#else

static NSMutableData *statframe=nil;
static void _send_bytes(uint8_t *b, int l)
{
    [statframe appendBytes:b length:l];
}
void txframe_send(frame_msg_t *m, int discardable)
{
    if (m->t == TXFRAME_TYPE_STAT) {
        statframe = [[NSMutableData alloc]initWithBytes:"|_NG\000X____" length:6];
        NSTimeInterval t = [NSDate timeIntervalSinceReferenceDate];
        uint32_t dt = 1000*(t-theDelegate->t0);
        frame_send_stat(_send_bytes, dt);
        _send_bytes((uint8_t *)"|", 1);
        if ((TRC)) NSLog(@"from simu stat frame %d bytes : %@\n", (int)[statframe length], [theDelegate dumpFrames:statframe]);
        [theDelegate performSelectorOnMainThread:@selector(processFrames:) withObject:statframe waitUntilDone:NO];
        statframe = nil;
        return;
    }
    NSData *d = [NSData dataWithBytes:m->frm length:m->len];
    if ((TRC)) NSLog(@"from notif frame %d bytes : %@\n", (int)[d length], [theDelegate dumpFrames:d]);
    [theDelegate performSelectorOnMainThread:@selector(processFrames:) withObject:d waitUntilDone:NO];
}
#endif

int cur_freqhz = 50;
void set_pwm_freq(int freqhz)
{
    
}
/*

void notif_target_bemf(const train_config_t *cnf, train_vars_t *vars, int32_t val)
{
    theDelegate.target_bemf = (BEMF_RAW) ? val : val*1.0/MAX_PID_VALUE;
}
 */


#pragma mark - record and plot


- (IBAction) startRecord:(id)sender
{
    if (_recordState) return;
    NSURL * tdir = [[NSFileManager defaultManager]temporaryDirectory];
    NSString *s = self.recordName;
    if (!s) s = @"record";
    s = [s stringByAppendingString:@".csv"];
    NSURL *fu = [tdir URLByAppendingPathComponent:s];
    NSLog(@"file : %@", fu);
    NSError *err;
    [[NSFileManager defaultManager]createFileAtPath:[fu path] contents:nil attributes:nil];
    recordFile = [NSFileHandle fileHandleForWritingToURL:fu error:&err];
    if (!recordFile) {
        NSLog(@"error creating file : %@", err);
        return;
    }
    self.recordState=1;
    self.fileURL = fu;
    [self addLog:[NSString stringWithFormat:@"RECORD : %@\r\n", [fu path]] important:YES error:NO];
}
- (IBAction) markRecord:(id)sender
{
    if (!_recordState) return;
}
- (IBAction) stopRecord:(id)sender
{
    if (!_recordState) return;
    [recordFile closeFile];
    self.recordState = 0;
}
//@property (nonatomic, strong) NSString *recordName;


/*
 NSTask *gnuplotTask;
   NSPipe *gnuplotStdin;
 */

- (void) startGnuplot
{
    if (gnuplotTask) {
        NSLog(@"previous gnuplot rc=%d %d", gnuplotTask.terminationStatus,
              (int) gnuplotTask.terminationReason);
      /*  @property (readonly) int terminationStatus;
        @property (readonly) NSTaskTerminationReason terminationReason
  @property (nullable, copy) void (^terminationHandler)(NSTask *) API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);
*/
    }
    NSLog(@"start gnuplot");
    gnuplotTask = [[NSTask alloc] init];
    gnuplotStdin = [[NSPipe alloc] init];
    gnuplotStdout = [[NSPipe alloc] init];

    gnuplotStdinFh = [gnuplotStdin fileHandleForWriting];
    gnuplotStdoutFh = [gnuplotStdout fileHandleForReading];

    [gnuplotTask setLaunchPath:@"/opt/local/bin/gnuplot"];
    [gnuplotTask setCurrentDirectoryPath:@"/tmp"];

    [gnuplotTask setArguments:[NSArray array]];

    [gnuplotTask setStandardInput:gnuplotStdin];
    [gnuplotTask setStandardOutput:gnuplotStdout];
    [gnuplotTask setStandardError:gnuplotStdout];

     [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(readStdout:) name:NSFileHandleDataAvailableNotification  object:gnuplotStdoutFh];
    [gnuplotStdoutFh waitForDataInBackgroundAndNotify];

    // Launch the task
    NSError *err = nil;
    gnuplotTask.terminationHandler = ^(NSTask *t) {
        NSLog(@"terminated gnuplot rc=%d %d", t.terminationStatus,
              (int) t.terminationReason);
        NSData *d = [self->gnuplotStdoutFh availableData];
        if (d) {
            NSLog(@"out %@\n%@", [self dumpFrames:d],
                  [[NSString alloc]initWithData:d encoding:NSUTF8StringEncoding]);
        }

    };
    [gnuplotTask launchAndReturnError:&err];
    if (err) {
        NSLog(@"launch error : %@\n", err);
    }
}

- (void) readStdout:(NSNotification *)notif
{
    NSData *d;
    @try {
        d = [gnuplotStdoutFh availableData];
    } @catch (NSException *exception) {
        NSLog(@"exception %@", exception);
        d = nil;
    } @finally {
    }
    if (!d) {
        return;
    }
    NSString *s = [[NSString alloc]initWithData:d encoding:NSUTF8StringEncoding];
    NSLog(@"gnuplot: %@", s ? s : d);
}

- (void) plotWithGnuplot:(NSString *)gnuplotCmd
{
    if (![gnuplotTask isRunning]) [self startGnuplot];
    [gnuplotStdinFh writeData:[gnuplotCmd dataUsingEncoding:NSUTF8StringEncoding]];
}

- (NSString *) _buildGnuplotCurves:(NSArray *)col x:(int)numx file:(NSString *)file
{
    NSMutableString *res = [[NSMutableString alloc]initWithCapacity:400];
    BOOL wl = numx ? NO : YES;
    [col enumerateObjectsUsingBlock:^(id colname, NSUInteger idx,  BOOL *stop) {
        NSNumber *nnumcol = [recordItems objectForKey:colname];
        if (!nnumcol) return;
        char *virg = ([res length]) ? "," : "plot";
        NSString *title = [colname stringByReplacingOccurrencesOfString:@"_" withString:@" "];
        [res appendFormat:@"%s '%@' using %d:%d %s title \"%@\"", virg, file,numx+1,  [nnumcol intValue]+1, wl ? "with lines" : "",
             title];
    }];
    return res;
}

- (NSString *) buildGnuplotCmd:(NSArray *)col x:(NSString *)colx file:(NSString *)file
{
    int numx = 0;
    NSString *namex = @"t";
    if (colx) {
        numx = [[recordItems objectForKey:colx]intValue];
        namex = colx;
    }
    NSMutableString *res = [[NSMutableString alloc]initWithCapacity:400];
    [res appendFormat:@"set terminal aqua\r\nset xlabel \"%@\"\r\n", namex];
    [res appendString:[self _buildGnuplotCurves:col x:numx file:file]];
    [res appendString:@"\r\n"];
    NSLog(@"gnuplot command :\n%@\n", res);
    return res;
}

- (void) gnuplot:(NSArray *)col x:(NSString *)colx file:(NSString *)file
{
    NSString *cmd = [self buildGnuplotCmd:col x:colx file:file];
    [self plotWithGnuplot:cmd];
    
}



- (void) gnuplotGraph:(int)ngraph
{
    static NSDictionary *graphs = nil;
    if (!graphs) graphs =
    @{ @"power"  : @[ @"", @"target_speed", @"curspeed", @"canton_0_centivolts", @"canton_0_pwm", @"vidx"],
       @"power2" : @[ @"curspeed", @"canton_0_centivolts", @"canton_0_pwm", @"vidx"],
       @"BEMF"   : @[ @"",  @"canton_0_bemfcentivolt", @"bemfiir_centivolts"/*,  @"pid_sum_e"*/],
       @"Vsense" : @[ @"", @"canton_0_bemf_centivolt", @"canton_0_von_centivolt"],

       @"PID"    : @[ @"", @"pid_target", @"canton_0_bemfcentivolt", @"pid_last_err", @"bemfiir_centivolts"/*,  @"pid_sum_e"*/],
       @"inertia": @[@"ine_t", @"ine_c"],
       @"pose"   : @[@"curspped", @"canton_0_bemfcentivolt", @"dir", @"train0_pose"]
    };
    NSString *k = nil;
    switch (ngraph) {
        case 0: k=@"power"; break;
        case 1: k=@"power2"; break;
        case 2: k=@"BEMF"; break;
        case 3: k=@"PID"; break;
        case 4: k=@"inertia"; break;
        case 5: k=@"pose"; break;
        default: return;
    }
    NSArray *t = [graphs objectForKey:k];
    if (!t) return;
    NSString *colx = [t objectAtIndex:0];
    if (![colx length]) colx = nil;
    NSString *f = [self.fileURL path];
    [self gnuplot:[t subarrayWithRange:NSMakeRange(1, [t count]-1)] x:colx file:f];
    
}
- (IBAction) plotRecord:(id)sender
{
    [self gnuplotGraph:self.plotNum];
}




#pragma mark - BLE

- (IBAction) startBLE:(id)sender
{
    if (!cbcentral) [self _startBLE];
}
- (void)_startBLE
{
    connected = NO;
    cbcentral = [[CBCentralManager alloc] initWithDelegate:self queue:nil options:nil];
    NSAssert(cbcentral, @"no cbcentral");
    //[cbcentral scanForPeripheralsWithServices:nil options:nil];
}

- (void)centralManager:(CBCentralManager *)central
didDiscoverPeripheral:(CBPeripheral *)peripheral
    advertisementData:(NSDictionary *)advertisementData
                 RSSI:(NSNumber *)RSSI
{

   NSLog(@"Discovered %@", peripheral.name);
    // identifier = E6DF4001-59B0-45F2-A31C-BBE4DE58B07B
    if (([peripheral.name isEqual:@"trainctl"])) {
        NSLog(@"connecting\n");
        trainctlBle = peripheral;
        trainctlBle.delegate = self;
        connected = NO;
        [self reconnectBLE];
        [cbcentral stopScan];
    }
}

- (void) connectionTimeout
{
    NSLog(@"connectionTimeout state=%d", (int) trainctlBle.state);
    [connectTimeout invalidate];
    if (connected) {
        NSLog(@"problem here");
    }
    [cbcentral cancelPeripheralConnection:trainctlBle];
    [self performSelector:@selector(reconnectBLE) withObject:nil afterDelay:1];
}

- (void) reconnectBLE
{
    NSAssert(trainctlBle, @"no periph");
    NSAssert(cbcentral, @"no cbcentral");
    NSAssert(!connected, @"already connected");
    NSLog(@"connecting BLE...");
    connectTimeout = [NSTimer timerWithTimeInterval:15.0 target:self selector:@selector(connectionTimeout) userInfo:nil repeats:NO];
    [[NSRunLoop mainRunLoop]addTimer:connectTimeout forMode:NSDefaultRunLoopMode];
    [cbcentral connectPeripheral:trainctlBle options:nil];
}
- (void)centralManager:(CBCentralManager *)central
 didConnectPeripheral:(CBPeripheral *)peripheral
{
    NSLog(@"Peripheral connected");
    [connectTimeout invalidate];
    connected = YES;
    //peripheral.delegate = self;
    [peripheral discoverServices:nil];
}

- (void)centralManager:(CBCentralManager *)central
didDisconnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error
{
    [connectTimeout invalidate];
    connected = NO;
}
- (void)centralManager:(CBCentralManager *)central
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                 error:(NSError *)error
{
    [connectTimeout invalidate];
    connected = NO;
}


- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
        case CBManagerStatePoweredOn:
            NSLog(@"power on");
            [cbcentral scanForPeripheralsWithServices:@[[CBUUID UUIDWithString:@"FFE0"]] options:nil];
            break;
        default:
            NSLog(@"state %d", (int) central.state);
            break;
    }
}

/*
- (void)centralManager:(CBCentralManager *)central
willRestoreState:(NSDictionary<NSString *,id> *)dict
{
    
}
 
*/


- (void)peripheral:(CBPeripheral *)peripheral
didDiscoverServices:(NSError *)error
{
 
    for (CBService *service in peripheral.services) {
        NSLog(@"Discovered service %@", service);
        if ([service.UUID isEqual:[CBUUID UUIDWithString:@"FFE0"]]) {
            [peripheral discoverCharacteristics:nil forService:service];
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
didDiscoverCharacteristicsForService:(CBService *)service
             error:(NSError *)error
{
    NSArray *cs = service.characteristics;
    for (CBCharacteristic *c in cs) {
        if ((1)) {
            [peripheral setNotifyValue:YES forCharacteristic:c];
        }
        [peripheral readValueForCharacteristic:c];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral
didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error
{
 
    NSData *data = characteristic.value;
    NSLog(@"characteristic  %@ val %@ %@\n",  characteristic, data, [[NSString alloc]initWithData:data encoding:NSUTF8StringEncoding]);
}

- (void)peripheral:(CBPeripheral *)peripheral
didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
             error:(NSError *)error
{
    if (error) {
        NSLog(@"Error changing notification state: %@",
              [error localizedDescription]);
    }
    NSData *data = characteristic.value;
    NSLog(@"notif characteristic  %@ val %@ %@\n",  characteristic, data,  [[NSString alloc]initWithData:data encoding:NSUTF8StringEncoding]);
}


@end
