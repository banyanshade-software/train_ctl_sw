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
//#include "txrxcmd.h"
//#include "low/canton.h"
#include "StringExtension.h"
#include "uitrack.h"
#include "topology.h"
#include "occupency.h"

#import "CTCManager.h"
#include "oscillo.h"

#include "conf_train.h"
#include "conf_train.propag.h"
#include "conf_canton.h"
#include "conf_canton.propag.h"
#include "conf_turnout.h"
#include "conf_turnout.propag.h"
#include "conf_globparam.h"
#include "conf_globparam.propag.h"
#include "framing.h"
#include "oam.h"
#include "conf_canton.h"
#include "canton_bemf.h"

uint16_t dummy[3];

const stat_val_t statval_ina3221[] = {
        { dummy, 0, 2       _P("ina0")},
        { dummy, 2, 2       _P("ina1")},
        { dummy, 4, 2       _P("ina2")},
        { NULL, 0, 0 _P(NULL)}
};

#define HIGHLEVEL_SIMU_CNX 0

/*
 * response format :
 * |s'R'rvv..|
 * notif format
 * |s'N'SNCvv..|
 */
#define MAX_DATA_LEN (1024*32)
typedef struct {
    uint8_t state;
    uint8_t escape;
    uint8_t seqnum;
    uint8_t notif;
    uint8_t msg64;
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
    int nextParamGet;
    NSMutableDictionary *parctl;
    
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

    NSTimer *usbTimer;

    // simu
    NSTimer *simuTimer;
    NSTimeInterval lastSimu;
    NSTimeInterval t0;
    SimTrain *simTrain0;
    
    // record
    NSFileHandle *recordFile;
    NSMutableDictionary *recordItems;
    NSMutableDictionary *cantons_value;
    NSMutableDictionary *trains_value;
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
    
    int t0changed, t1changed, t2changed, t3changed;
    
    // avoid exception in KVC
    BOOL processingStatFrame;
    BOOL undefinedKey;
}

@property (weak) IBOutlet NSWindow *window;
@property (nonatomic, readwrite) int curspeed;
@property (nonatomic, readwrite) int transmit;
@property (nonatomic, readwrite) int linkok;
@property (nonatomic, readwrite)  SimTrain *simTrain0;

@end

@implementation AppDelegate {
}

@synthesize curspeed = _curspeed;
@synthesize dspeedT0 = _dspeedT0;
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
    nparam = 0;
    [self forAllParamsDo:^(NSControl *c){
        c.enabled = NO;
        self->nparam++;
    }];
    self.numtrains = 1; //XXX obsolete
    self.numcantons = 1+1; // XXX obsolete
    
    [_ctcManager loadHtml];
    
    // for debug
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
    [self willChangeValueForKey:@"minslider"];
    [self willChangeValueForKey:@"maxslider"];
    self.sliderTarget1.minValue = -m;
    self.sliderTarget1.maxValue = m;
    self.sliderTarget2.minValue = -m;
    self.sliderTarget2.maxValue = m;
    self.sliderTarget3.minValue = -m;
    self.sliderTarget3.maxValue = m;
    [self didChangeValueForKey:@"minslider"];
    [self didChangeValueForKey:@"maxslider"];
    self.sliderTarget1.numberOfTickMarks= _shunting ? 7 : 21;
    self.sliderTarget2.numberOfTickMarks= _shunting ? 7 : 21;
    self.sliderTarget3.numberOfTickMarks= _shunting ? 7 : 21;
    if (_shunting) {
        if (_dspeedT0<-m) self.dspeedT0 = -m;
        if (_dspeedT0>m) self.dspeedT0 = m;
        if (_dspeedT1<-m) self.dspeedT1 = -m;
        if (_dspeedT1>m) self.dspeedT1 = m;
        if (_dspeedT2<-m) self.dspeedT2 = -m;
        if (_dspeedT3>m) self.dspeedT2 = m;
        if (_dspeedT3<-m) self.dspeedT3 = -m;
        if (_dspeedT3>m) self.dspeedT3 = m;
    }
}

- (int) minslider
{
    return  self.sliderTarget1.minValue;
}
- (int) maxslider
{
    return self.sliderTarget1.maxValue;
}

- (void) setPolarity:(int)p
{
    if (_polarity == p) return;
    _polarity = p;
    self.curspeed = - _curspeed;
    self.dspeedT0 = - _dspeedT0;
    self.dspeedT1 = - _dspeedT1;
    self.dspeedT2 = - _dspeedT2;
    self.dspeedT3 = - _dspeedT3;
}

- (void) goZero:(id)sender
{
    [self setDspeedT0:0];
    [self setDspeedT1:0];
    [self setDspeedT2:0];
    [self setDspeedT3:0];
}
- (void) setDspeedT0:(int)v
{
    if (!_linkok) return;
    if (_linkok < LINK_OK) return;
    NSLog(@"dspeedT0 %d", v);
    if (_dspeedT0==v) return;
    _dspeedT0 = v;
    t0changed = 1;
    [self sendSpeedOrCoalesce];
}
- (void) setDspeedT1:(int)v
{
    if (_linkok < LINK_OK) return;
    NSLog(@"dspeedT0 %d", v);
    if (_dspeedT1==v) return;
    _dspeedT1 = v;
    t1changed=1;
    [self sendSpeedOrCoalesce];
}
- (void) setDspeedT2:(int)v
{
    if (_linkok < LINK_OK) return;
    NSLog(@"dspeedT0 %d", v);
    if (_dspeedT2==v) return;
    _dspeedT2 = v;
    t2changed = 1;
    [self sendSpeedOrCoalesce];
}
- (void) setDspeedT3:(int)v
{
    if (_linkok < LINK_OK) return;
    NSLog(@"dspeedT0 %d", v);
    if (_dspeedT3==v) return;
    _dspeedT3 = v;
    t3changed = 1;
    [self sendSpeedOrCoalesce];
}

- (void) sendSpeedOrCoalesce
{
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
    if (t0changed) [self sendTargetSpeed:_dspeedT0 train:0];
    if (t1changed) [self sendTargetSpeed:_dspeedT1 train:1];
    if (t2changed) [self sendTargetSpeed:_dspeedT2 train:2];
    if (t3changed) [self sendTargetSpeed:_dspeedT3 train:3];
    t0changed = t1changed = t2changed = t3changed = 0;
}

- (void) sendTargetSpeed:(int)v train:(int)t
{
    int16_t v16 = (uint16_t) (_polarity * v);
    msg_64_t m;
    m.to = MA1_CTRL(t);
    m.from = MA3_UI_GEN;//(UISUB_USB);
    m.cmd = CMD_MDRIVE_SPEED_DIR;
    m.v1u = abs(v16);
    m.v2 = SIGNOF0(v16);
    [self sendMsg64:m];
}

- (IBAction) stopAll:(id)sender
{
    if ([timSendThrottle isValid]) {
        NSLog(@"...");
    }
    [timSendThrottle invalidate];
    timSendThrottle = nil;
    if (_linkok == LINK_SIMUHI) {
        //train_stop_all();
    } else {
#if 0
        uint8_t spdfrm[] = "|zG\0S|....";
        int l = 2+4+0;
        [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
            self.curspeed = 0;
        }];
#endif
        msg_64_t m = {0};
        m.from = MA3_UI_GEN;//(0);
        m.to = MA3_BROADCAST;
        m.cmd = CMD_EMERGENCY_STOP;
        [self sendMsg64:m];
    }
    [self willChangeValueForKey:@"dspeedT0"];
    [self willChangeValueForKey:@"dspeedT1"];
    [self willChangeValueForKey:@"dspeedT2"];
    [self willChangeValueForKey:@"dspeedT3"];
    _dspeedT0 = 0;
    _dspeedT1 = 0;
    _dspeedT2 = 0;
    _dspeedT3 = 0;
    [self didChangeValueForKey:@"dspeedT0"];
    [self didChangeValueForKey:@"dspeedT1"];
    [self didChangeValueForKey:@"dspeedT2"];
    [self didChangeValueForKey:@"dspeedT3"];
}



- (IBAction) startAuto:(id)sender
{
    msg_64_t m = {0};
    m.to = MA1_CONTROL();
    m.from = MA3_UI_GEN;//(0);
    m.cmd = CMD_START_AUTO;
    m.v1u = _autoNum;
    [self sendMsg64:m];
}

- (IBAction) sendLed:(id)sender
{
    int tag = (int) [sender tag];
    int ledNum = 0;
    int brdNum = 0;
    switch (tag) {
        default:
        case 0:
            ledNum = 0; brdNum = 0;
            break;
        case 1:
            ledNum = 0; brdNum = 1;
            break;
    }
    msg_64_t m = {0};
    m.to = MA0_LED(brdNum);
    m.from = MA3_UI_GEN; //(0);
    m.cmd = CMD_LED_RUN;
    m.v1u = ledNum;
    m.v2u = _ledProg;
    [self sendMsg64:m];
}

- (IBAction) turnoutA:(id)sender
{
    NSControl *c = (NSControl *)sender;
    NSInteger tn = c.tag;

    msg_64_t m;
    m.to = MA1_CONTROL(); //MA_TURNOUT(0, 0);
    m.from = MA3_UI_GEN; //(UISUB_USB);
    m.cmd = CMD_TURNOUT_HI_A;
    m.v1u = (uint16_t) tn;
    [self sendMsg64:m];
}

- (IBAction) turnoutB:(id)sender
{
    NSControl *c = (NSControl *)sender;
    NSInteger tn = c.tag;
    msg_64_t m;
    m.to = MA1_CONTROL(); //MA_TURNOUT(0, 0);
    m.from = MA3_UI_GEN; //(UISUB_USB);
    m.cmd = CMD_TURNOUT_HI_B;
    m.v1u = (uint16_t) tn;
    [self sendMsg64:m];
}

- (void) toggleTurnout:(int)tn
{
    msg_64_t m;
    m.to = MA1_CONTROL(); //MA_TURNOUT(0, 0);
    m.from = MA3_UI_GEN; //(UISUB_USB);
    m.cmd = CMD_TURNOUT_HI_TOG;
    m.v1u = (uint16_t) tn;
    [self sendMsg64:m];
}


- (IBAction)clearePose:(id)sender
{
    /*
    uint8_t spdfrm[] = "|zT\0z|....";
    //NSInteger t = [sender tag];
    //spdfrm[3] = (uint8_t)t;
    int l = 2+4+0;
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:^{
        NSLog(@"clearePose done");
    }];
     */
}

- (IBAction) setTrainMode:(id)sender
{
    NSControl *ct = (NSControl *)sender;
    int tr = (int) ct.tag;
    int v = ct.intValue;
    msg_64_t m = {0};
    m.cmd = CMD_SET_TRAIN_MODE;
    m.v1u = tr;
    m.v2u = v;
    m.from = MA3_UI_GEN; //(0);
    m.to = MA1_CONTROL();
    [self sendMsg64:m];
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
    //railconfig_setup_default();
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
    
    
    NSArray *pa = [self splitParamName:x];
    NSString *psel = [pa objectAtIndex:1];
    NSString *pn = [pa objectAtIndex:2];
    if ([psel length] != 2) {
        return;
    }
    const char *cpsel = [psel cStringUsingEncoding:NSUTF8StringEncoding];
    const char *cpn = [pn cStringUsingEncoding:NSUTF8StringEncoding];
    
    int confnum = -1;
    int instnum = cpsel[1]-'0';
    int fieldnum = -1;
    int boardnum = 0;
    
    
    int loc = 0;
    switch (cpsel[0]) {
        case 'T':
            loc = 1;
            confnum = conf_lnum_train;
            /*if (!strcmp(cpn, "ki")) cpn = "kI";
             if (!strcmp(cpn, "kp")) cpn = "kP";
             if (!strcmp(cpn, "kd")) cpn = "kD";
             if (!strcmp(cpn, "en_pid")) cpn = "enable_pid";*/
            fieldnum = conf_train_fieldnum(cpn);
            break;
        case 'G':
            loc = 1;
            confnum = conf_lnum_globparam;
            fieldnum = conf_globparam_fieldnum(cpn);
            break;
        case 't':
            confnum = conf_pnum_turnout;
            fieldnum = conf_turnout_fieldnum(cpn);
            break;
        case 'C':
            confnum = conf_pnum_canton;
            fieldnum = conf_canton_fieldnum(cpn);
            break;
        default:
            NSLog(@"bad param def");
            break;
    }
    if ((confnum<0)||(fieldnum<0)) {
        return;
    }
    uint64_t v40;
    oam_encode_val40(&v40, confnum, boardnum, instnum, fieldnum, v);
    msg_64_t m = {0};
    m.to = MA0_OAM(0);
    m.from = MA3_UI_GEN;
    m.cmd = loc ? CMD_PARAM_LUSER_SET : CMD_PARAM_USER_SET;
    m.val40 = v40;
    [self sendMsg64:m];
    
#if 0
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
#endif
}

- (IBAction)commitTrains:(id)sender
{
    msg_64_t m = {0};
    m.to = MA0_OAM(0);
    m.from = MA3_UI_GEN;
    m.cmd = CMD_PARAM_LUSER_COMMIT;
    m.v1 = conf_lnum_train;
    [self sendMsg64:m];
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

    [self getParams:0];
}

// TODO conf generator does not produce .h
int conf_canton_fieldnum(const char *str);
int conf_turnout_fieldnum(const char *str);
int conf_train_fieldnum(const char *str);
int conf_globparam_fieldnum(const char *str);

- (void) getParams:(int)np1
{
    nextParamGet = np1;
    [self _getParams];
}
- (void) _getParams
{
    int np1 = nextParamGet;
    int np2 = nextParamGet+4;
    parctl = [NSMutableDictionary dictionaryWithCapacity:5];
    //static uint8_t gpfrm[80] = "|xT\0p......";
    int __block numparam = -1;
    nextParamGet = np2;
    [self forAllParamsDo:^(NSControl *c){
        numparam++;
        if (numparam < np1) return;
        if (numparam >= np2) return;
        NSLog(@"getParam %d", numparam);
        
        NSString *s = c.identifier;
        self->parctl[s] = c;
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
        
        int confnum = -1;
        int instnum = cpsel[1]-'0';
        int fieldnum = -1;
        int boardnum = 0;
        
        
        int loc = 0;
        switch (cpsel[0]) {
            case 'T':
                loc = 1;
                confnum = conf_lnum_train;
                /*if (!strcmp(cpn, "ki")) cpn = "kI";
                if (!strcmp(cpn, "kp")) cpn = "kP";
                if (!strcmp(cpn, "kd")) cpn = "kD";
                if (!strcmp(cpn, "en_pid")) cpn = "enable_pid";*/
                fieldnum = conf_train_fieldnum(cpn);
                break;
            case 'G':
                loc = 1;
                confnum = conf_lnum_globparam;
                fieldnum = conf_globparam_fieldnum(cpn);
                break;
            case 't':
                confnum = conf_pnum_turnout;
                fieldnum = conf_turnout_fieldnum(cpn);
                break;
            case 'C':
                confnum = conf_pnum_canton;
                fieldnum = conf_canton_fieldnum(cpn);
                break;
            default:
                NSLog(@"bad param def");
                break;
        }
        if ((confnum<0)||(fieldnum<0)) {
            return;
        }
        
        uint64_t v40;
        oam_encode_val40(&v40, confnum, boardnum, instnum, fieldnum, 0);
        msg_64_t m = {0};
        m.to = MA0_OAM(0);
        m.from = MA3_UI_GEN;
        m.cmd = loc ? CMD_PARAM_LUSER_GET : CMD_PARAM_USER_GET;
        m.val40 = v40;
        [self sendMsg64:m];
        
#if 0
        
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
        NSLog(@"N=%d get param %c%c '%s'\n",  n, cpsel[0], cpsel[1], cpn);
    
        if ((0) && (0==(numparam%5))) {
            //sleep(1);
            usleep(800);
        }
        [self sendFrame:gpfrm len:(int)(5+nl+2) blen:sizeof(gpfrm) then:^{
            // handle response
            self->nparamresp++;
            NSLog(@"param %d resp %d", self->nparam, self->nparamresp);
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
#endif
        //NSLog(@"hop");
    }];
}

- (void) paramUserVal:(msg_64_t)m locstore:(int)loc
{
    self->nparamresp++;
    if (self->nparamresp==self->nparam) {
        if (self.linkok<LINK_OK) self.linkok = LINK_OK;
    }
    if (self->nparamresp == nextParamGet) {
        [self performSelectorOnMainThread:@selector(_getParams) withObject:NULL waitUntilDone:NO];
    }
    
    unsigned int fnum; unsigned int brd; unsigned int inst; unsigned int field; int32_t v;
    oam_decode_val40(m.val40, &fnum, &brd, &inst, &field, &v);
    NSLog(@"paramUserVal");
    
    // proto not generated
    const char *conf_train_fieldname(int f);
    const char *conf_globparam_fieldname(int f);
    const char *conf_boards_fieldname(int f);
    const char *conf_canton_fieldname(int f);

    char t;
    int n = inst > 9 ? '-' : inst;
    const char *fld;
    
    if (loc) {
        switch (fnum) {
            case conf_lnum_train:
                t = 'T';
                fld = conf_train_fieldname(field);
                break;
            case conf_lnum_globparam: //XXX loc
                t = 'G';
                fld = conf_globparam_fieldname(field);
                break;
            default:
                NSLog(@"bad conf num in paramUserVal");
                return;
                break;
        }
    } else {
        switch (fnum) {
            case conf_pnum_canton:
                t = 'C';
                fld = conf_canton_fieldname(field);
                break;
            case conf_pnum_turnout: //XXX loc
                t = 't';
                fld = conf_turnout_fieldname(field);
                break;
            default:
                NSLog(@"bad conf num in paramUserVal");
                return;
                break;
        }
    }
    NSString *pnam = [NSString stringWithFormat:@"par_%c%d_%s", t, n, fld];
    NSLog(@"val for '%@'", pnam);
    NSControl *c = parctl[pnam];
    [self setParameter:c value:v enable:YES];
}

- (void) setParameter:(NSControl *)c value:(int)v /*def:(int)def min:(int)min max:(int)max*/ enable:(BOOL)ena
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
    /*
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
    */
    if (ena) c.enabled = YES;
}

#if 0
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
            [self setParameter:c value:def /*def:def min:min max:max*/ enable:YES];
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
            [self setParameter:c value:0 /*def:def min:min max:max*/ enable:YES];
            NSLog(@"val %d def %d min %d max %d\n", val, def ,min, max);
            [self changeParam:c];
        }];
    }];
}
#endif

- (IBAction) defaultInertia:(id)sender
{
   // [self resetToDefault:@[@"par_T0_acc", @"par_T0_dec"]];
}
- (IBAction) zeroInertia:(id)sender
{
   // [self resetToZero:@[@"par_T0_acc", @"par_T0_dec"]];
}
- (IBAction) defaultPwmMinMax:(id)sender
{
   // [self resetToDefault:@[@"par_T0_minpwm", @"par_T0_maxpwm"]];
}

- (IBAction) startCalibration:(id)sender
{
    /*uint8_t spdfrm[] = "|xG\0K|.........";
    int l = 2+4+0;
    uint16_t v16 = (uint16_t) (_polarity * _dspeedT0);
    if (_linkok == LINK_SIMUHI) {
        return;
    }
    // assume same endienness (little)
    memcpy(spdfrm+5, &v16, 2);
    [self sendFrame:spdfrm len:l blen:sizeof(spdfrm) then:nil];
*/
}

- (IBAction)sendOamCustom:(id)sender
{
    msg_64_t m = {0};
    m.from = MA3_UI_GEN;
    m.to = MA0_OAM(_oamCustomBrd);
    m.cmd = CMD_OAM_CUSTOM;
    m.subc = _oamCustomCmd;
    [self sendMsg64:m];
}

#pragma mark -

#define USBTTY1 "/dev/cu.usbmodem6D94487754571"
#define USBTTY2 "/dev/cu.usbmodem376A356634381"
#define USBTTY3 "/dev/cu.usbmodem3158378430391"

- (void) openUsb
{
    if ((_linkok == LINK_SIMUHI) || (_linkok == LINK_SIMULOW)) return;
    retry++;
    const char *dev = USBTTY1;
    switch (retry%3) {
        default:
        case 0: dev=USBTTY1; break;
        case 1: dev=USBTTY2; break;
        case 2: dev=USBTTY3; break;
    }
    int fd = open(dev, O_RDWR|O_NOCTTY);
    if (fd<0) {
        self.linkok = NO;
        usb = nil;
        if (errno != ENOENT) {
            perror("open:");
        }
        [self performSelector:@selector(openUsb) withObject:nil afterDelay:1.0];
        return;
    }

    usb = [[NSFileHandle alloc]initWithFileDescriptor:fd closeOnDealloc:YES];
    //[usb readInBackgroundAndNotify];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(readUsbTty:) name:NSFileHandleDataAvailableNotification  object:usb];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(exceptionUsbTty:) name:NSFileHandleOperationException  object:usb];
    [usb waitForDataInBackgroundAndNotify];
    NSLog(@"connected");
    //[self performSelector:@selector(goFrame) withObject:nil afterDelay:0.5];
    [self goFrame];
}

- (void) goFrame
{
    NSLog(@"go frame\n");
    //nparam = 0;
    nparamresp = 0;
    self.transmit = 0;

    self.linkok = LINK_FRM;
    //self.linkok = LINK_OK; // XXX
    [self performSelector:@selector(getParams) withObject:nil afterDelay:0.2];
    //[self performSelector:@selector(getParams) withObject:nil afterDelay:2.2];
    
    theDelegate = self;
    simuTimer = [NSTimer timerWithTimeInterval:0.01 target:self selector:@selector(usbTimer) userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop]addTimer:simuTimer forMode:NSDefaultRunLoopMode];

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
   
    [self processUsbData:d];
    [usb waitForDataInBackgroundAndNotify];
}

- (void) exceptionUsbTty:(NSNotification *)not
{
    NSLog(@"exceptionUsbTty:");
}

- (IBAction)forceLinkOk:(id)sender
{
    if (!_linkok) return;
    if (_linkok == LINK_OK) return;
    self.linkok = LINK_OK;
}

- (void) processUsbData:(NSData *)d
{
    static uint8_t buf8[8];
    static int ptr8 = -1;
    static int frmstate = -1;
    static int frmtype = 0;
    static NSMutableData *frm = nil;
    
    int l = (int) [d length];
    const uint8_t *dta = [d bytes];
    for (int i=0; i<l; i++) {
        uint8_t c = dta[i];
        if (ptr8 >= 0) {
            buf8[ptr8++] = c;
            if (8==ptr8) {
                msg_64_t m;
                memcpy(&m, buf8, 8);
                ptr8 = -1;
                [self processMsg64:m];
            }
            continue;
        }
        if (frmstate < 0) {
            if (c == FRAME_M64) {
                ptr8 = 0;
                continue;
            } else if (c == FRAME_DELIM) {
                frmstate = 0;
                continue;
            } else {
                // ignore car
                continue;
            }
        }
        switch (frmstate) {
            case 0:
                frmtype = c;
                frm = [NSMutableData dataWithCapacity:1024];
                frmstate++;
                break;
            case 1:
                if (c==FRAME_DELIM) {
                    // frame complete
                    [self processFrame:frm type:frmtype];
                    frm = nil;
                    frmstate = -1;
                    break;
                } else if (c==FRAME_ESC) {
                    frmstate = 2;
                } else {
                    // normal char, add to frame
                    [frm appendBytes:&c length:1];
                }
                break;
            case 2:
                [frm appendBytes:&c length:1];
                frmstate = 1;
                break;
        }
    }
}

- (void) processMsg64:(msg_64_t)m
{
    if (MA3_UI_CTC==m.to) {
        mqf_write_to_ui_track(&m);
        return;
    }
    int nt, v;
    switch (m.cmd) {
    case CMD_NOTIF_SPEED:
        nt = MA1_TRAIN(m.from);
        v = m.v1u;
        NSLog(@"train %d spd %d\n", nt, v);
        self.curspeed = v;
        break;
    case CMD_PARAM_USER_VAL:
        [self paramUserVal:m locstore:0];
        break;
    case CMD_PARAM_LUSER_VAL:
        [self paramUserVal:m locstore:1];
        break;
    case CMD_SETRUN_MODE:
    case CMD_TRSTATE_NOTIF:
    case CMD_TRTSPD_NOTIF:
    case CMD_CANTEST:
        break;
    default:
        NSLog(@"frameMsg64 UI msg not handled 0x%X", m.cmd);
        break;
    }
}

- (void) processFrame:(NSData *)frm type:(int)frmtype
{
    switch (frmtype) {
        case 'S':
            [self processStatFrame:frm];
            break;
        case 'V':
            [self processOscilloFrame:frm];
            break;
    }
}


#if 0 // OLD_FRAMING

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
                } else if (frm.msg64) {
                    [self frameMsg64];
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
                } else if ('6'==c) {
                    frm.msg64 = 1;
                    frm.state = 6;
                    NSLog(@"msg6");
                } else {
                    NSLog(@"unknown msg");
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


- (void) frameMsg64
{
    if (frm.pidx != 8) {
        NSLog(@"frameMsg64 bad len");
        return;
    }
    if ((MA3_UI_GEN != frm.param[0]) && (MA3_UI_CTC != frm.param[0])) {
        NSLog(@"only handle UI");
    }
    int nt=0;
    int16_t v;
    msg_64_t m;
    memcpy(&m, frm.param, sizeof(m));

    if (MA3_UI_CTC==m.to) {
        mqf_write_to_ui_track(&m);
        return;
    }
    switch (m.cmd) {
        case CMD_NOTIF_SPEED:
            nt = MA1_TRAIN(m.from);
            v = m.v1u;
            NSLog(@"train %d spd %d\n", nt, v);
            self.curspeed = v;
            break;
        case CMD_PARAM_USER_VAL:
            [self paramUserVal:m locstore:0];
            break;
        case CMD_PARAM_LUSER_VAL:
            [self paramUserVal:m locstore:1];
            break;
        case CMD_SETRUN_MODE:
        case CMD_TRSTATE_NOTIF:
        case CMD_TRTSPD_NOTIF:
        case CMD_CANTEST:
            break;
        default:
            NSLog(@"frameMsg64 UI msg not handled 0x%X", m.cmd);
            break;
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
                case 'Y': {
                    // oscilo frame
                    [self processOscilloFrame];
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
                        self.T0_bemf_mv =  v[0]; // BEMF_RAW ?  : v[0]/100.0;
                        // IIR low pass on bemf
                       // self.canton_0_bemf_lp = _canton_0_bemf_lp*.97+0.03*_canton_0_bemfcentivolt;
                        if (frm.cmd == 'B') {
                            //self.canton_0_centivon =   BEMF_RAW ? v[1] : v[1]/100.0;
                            //self.canton_0_centivolts = v[2]/100.0;
                            self.C0_pwm =   v[3];
                        }
                    }
                    return;
            }
        case 'T': {
            //struct spd_notif spd;
            switch (frm.cmd) {
                case 'V':
                    /*
                    memcpy(&spd, frm.param, sizeof(spd));
                    //NSLog(@"train %d, v=%d\n", frm.num, spd.sv100);
                    if (0==frm.num || '0'==frm.num) { // XXX for test
                        self.curspeed = _polarity*spd.sv100;
                        //self.target_bemf = (BEMF_RAW) ? spd.pid_target: spd.pid_target/100.0;
                        self.target_bemf = spd.pid_target;
                        if ((1)) {
                            static int bmin = 0;
                            static int bmax = 0;
                            if (spd.bemf_centivolt > bmax) bmax = spd.bemf_centivolt;
                            if (spd.bemf_centivolt < bmin) bmin = spd.bemf_centivolt;
                            //NSLog(@"bemf %d [%d %d]", spd.bemf_centivolt, bmin, bmax);
                        }
                        self.train_bemf = spd.bemf_centivolt/100.0;
                       
                    }
                     */
                    return;
                    break;
                case 'i':
                    memcpy(&v32, frm.param, sizeof(int32_t));
                    self.T0_pose = v32;
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

#endif // OLD_FRAMING





- (void) setValue:(id)value forUndefinedKey:(NSString *)key
{
    if (processingStatFrame) {
        undefinedKey = YES;
        return;
    }
    [super setValue:value forUndefinedKey:key];
}
- (void) processStatFrame:(NSData *)dta
{
    NSUInteger len = [dta length];
    if (len<=8) {
        return; // TODO
    }
    NSAssert(len>8, @"short stat frame");
    //if ((1)) return;
    
    cantons_value = [[NSMutableDictionary alloc]initWithCapacity:25];
    trains_value = [[NSMutableDictionary alloc]initWithCapacity:25];

    NSUInteger nval = len / 4;
    uint32_t *ptr = frm.param32;
    //uint32_t tick = *ptr++;
    nval--;
    stat_iterator_t step;
    int rc;
    int validx;
    for (validx=0,rc = stat_iterator_reset(&step); !rc; rc = stat_iterator_next(&step), validx++) {
        int32_t v = *ptr++;
        off_t offset; int len;
        int idx=-1;
        const char *name = NULL;

        get_val_info(&step, &offset, &len, &idx, &name);
        
        if (nval<0) {
            NSLog(@"short stat frame");
            goto done;
        }
        //if (!strcmp(name, "C#_pwm")) NSLog(@"stat : '%s'[%d] = %d\n", name ? name : "_", idx, v);
        NSString *key = nil;

        static NSMutableSet *unhandled_key = nil;
        if (!unhandled_key) unhandled_key = [[NSMutableSet alloc]initWithCapacity:20];
        NSString *nkey;

        if (!name) {
            NSLog(@"no stat name");
            continue;
        }
        
        NSString *m = [NSString stringWithUTF8String:name];
        key = m;
        key = [NSString stringWithFormat:m, idx];
        
        NSString *sidx = [NSString stringWithFormat:@"%d", idx];
        nkey= [key stringByReplacingOccurrencesOfString:@"#" withString:sidx];
        
        if ([key isEqualToString:@"T#_pid_sum_e"]) {
            v = v/1000;
        }
        NSNumber *nsv = @(v);

        if ([key hasPrefix:@"C#_"]) {
            [cantons_value setValue:nsv forKey:nkey];
        } else if ([key hasPrefix:@"T#_"]) {
            [trains_value setValue:nsv forKey:nkey];
        }
        if (![unhandled_key containsObject:nkey]) {
            //NSLog(@"---- %@\n", key);
            processingStatFrame = YES;
            undefinedKey = NO;
            [self setValue:nsv forKey:nkey];
            if (undefinedKey) {
                [unhandled_key addObject:nkey];
            }
            processingStatFrame = NO;

            /*
            @try {
                [self setValue:nsv forKey:nkey];
            } @catch (NSException *exception) {
                NSLog(@"key %@ (%@) not ok: %@", nkey, key, exception);
                [unhandled_key addObject:nkey];
            }*/
        }
        
        if (_recordState == 1) {
            if (!recordItems) recordItems=[[NSMutableDictionary alloc]init];
            
            if (nkey) [recordItems setObject:@(validx) forKey:nkey];
            NSString *s = [NSString stringWithFormat:@"%@, ", nkey ? nkey : @"_"];
            [recordFile writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
        } else if (_recordState) {
            NSString *s;
            s = [NSString stringWithFormat:@"%d, ", v];
            [recordFile writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
        }
    }
done:
    if (_recordState) {
        [recordFile writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];
        _recordState = 2;
    }
    [self.cantonTableView reloadData];
    [self.trainTableView reloadData];
}

int convert_to_mv(int m)
{
    return ((m * 4545 * 33) / (4096*10));
}
int convert_to_mv_raw(int m)
{
    return m; // dont convert for raw file
}

- (NSURL *) createTempCsvFile:(NSString *)basename second:(NSString *)secname retfile:(NSFileHandle **)retfile
{
    if (retfile) *retfile = nil;
    NSURL * tdir = [[NSFileManager defaultManager]temporaryDirectory];
    NSString *s = basename;
    if (secname) {
        s = [s stringByAppendingFormat:@".%@", secname];
    }
    s = [s stringByAppendingString:@".csv"];
    NSURL *fu = [tdir URLByAppendingPathComponent:s];
    NSLog(@"file : %@", fu);
    if (retfile) {
        NSError *err;
        [[NSFileManager defaultManager]createFileAtPath:[fu path] contents:nil attributes:nil];
        NSFileHandle *ref = [NSFileHandle fileHandleForWritingToURL:fu error:&err];
        if (!ref) {
            NSLog(@"error creating file : %@", err);
        }
        *retfile = ref;
    }
    return fu;
}

volatile int oscillo_trigger_start = 0;
volatile int oscillo_enable = 0;

- (void) processOscilloFrame:(NSData *)dta
{
    NSUInteger len = [dta length];
    if (len<8) return; // TODO
    int rs = len % sizeof(osc_values_t);
    NSAssert(!rs, @"bad size");
    NSUInteger ns = len / sizeof(osc_values_t);
    NSAssert(ns == OSC_NUM_SAMPLES, @"bad size 2");
    osc_values_t *samples = (osc_values_t *) [dta bytes];
    
    NSFileHandle *recordFileGp;
    NSFileHandle *recordFileRaw;
    NSURL *fugp = [self createTempCsvFile:@"oscilo" second:@"gnup" retfile:&recordFileGp];
     if (!recordFileGp) {
        return;
    }
    NSURL *furaw = [self createTempCsvFile:@"oscilo" second:@"raw" retfile:&recordFileRaw];
    
    
    NSArray *cols=@[@"V0", @"V1", @"V0a", @"V0b", @"V1a", @"V1b",
                    @"tim1", @"tim2", @"tim8",
                    @"T1ch1", @"T1ch2", @"T1ch3", @"T1ch4",
                    @"T2ch1", @"T2ch2", @"T2ch3", @"T2ch4",
                    @"t0bemf", @"t1bemf", @"evtadc", @"evtt1",
                    @"ina0", @"ina1", @"ina2"];
    NSDictionary *itms = @{ @"i" : @0,  @"V0" : @1 , @"V1" : @2 ,
                            @"V0a" : @3 , @"V0b" : @4 , @"V1a" : @5, @"V1b" : @6,
                            @"tim1" : @7, @"tim2" : @8, @"tim8" : @9,
        @"T1ch1" : @10, @"T1ch2" : @11, @"T1ch3" : @12, @"T1ch4" : @13,
        @"T2ch1" : @14, @"T2ch2" : @15, @"T2ch3" : @16, @"T2ch4" : @17,
        @"t0bemf" : @18, @"t1bemf" : @19, @"evtadc" : @20, @"evtt1" : @21,
        @"ina0" : @22, @"ina1" : @23, @"ina2" : @24
    };
    
    [recordFileRaw writeData:[@"i" dataUsingEncoding:NSUTF8StringEncoding]];
    for (NSString *c in cols) {
        NSString *s = [NSString stringWithFormat:@", %@", c];
        [recordFileRaw writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
    }
    [recordFileRaw writeData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];

#define W0(_n) (-6000-(_n)*1000)
#define W1(_n) (-6000-(_n)*1000+700)
    
    for (int i=0; i<ns; i++) {
        osc_values_t *v = &samples[i];
        NSString *s = [NSString stringWithFormat:@"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                       i,
                       convert_to_mv(v->vadc[0]-v->vadc[1])+10000*5,
                       convert_to_mv(v->vadc[2]-v->vadc[3])+10000*4,
                       convert_to_mv(v->vadc[0])+10000*3,
                       convert_to_mv(v->vadc[1])+10000*2,
                       convert_to_mv(v->vadc[2])+10000*1,
                       convert_to_mv(v->vadc[3])+10000*0,
                       v->tim1cnt*8 - 2000*2,
                       v->tim2cnt*8 - 2000*4,
                       v->tim8cnt*8 - 2000*8,
                       v->valt1ch1 ? W1(0) : W0(0),
                       v->valt1ch2 ? W1(1) : W0(1),
                       v->valt1ch3 ? W1(2) : W0(2),
                       v->valt1ch4 ? W1(3) : W0(3),
                       v->valt2ch1 ? W1(4) : W0(4),
                       v->valt2ch2 ? W1(5) : W0(5),
                       v->valt2ch3 ? W1(6) : W0(6),
                       v->valt2ch4 ? W1(7) : W0(7),
                       v->t0bemf +50000,
                       v->t1bemf -50000,
                       v->evtadc*2000 - 35000,
                       v->evtt1*2000 - 37000,
                       v->ina0+70000,
                       v->ina1+70000,
                       v->ina2+70000
                       ];
        [recordFileGp writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
        
        s = [NSString stringWithFormat:@"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                       i,
                       convert_to_mv_raw(v->vadc[0]-v->vadc[1]),
                       convert_to_mv_raw(v->vadc[2]-v->vadc[3]),
                       convert_to_mv_raw(v->vadc[0]),
                       convert_to_mv_raw(v->vadc[1]),
                       convert_to_mv_raw(v->vadc[2]),
                       convert_to_mv_raw(v->vadc[3]),
                       v->tim1cnt,
                       v->tim2cnt,
                       v->tim8cnt,
                       v->valt1ch1,
                       v->valt1ch2,
                       v->valt1ch3,
                       v->valt1ch4,
                       v->valt2ch1,
                       v->valt2ch2,
                       v->valt2ch3,
                       v->valt2ch4,
                       v->t0bemf,
                       v->t1bemf,
                       v->evtadc,
                       v->evtt1,
                       v->ina0, v->ina1, v->ina2
                       ];
        [recordFileRaw writeData:[s dataUsingEncoding:NSUTF8StringEncoding]];
    }
    [recordFileGp closeFile];
    [recordFileRaw closeFile];
    
    [self gnuplot:cols x:@"i" file:[fugp path] items:itms];
    NSLog(@"RAW file : %@", furaw);
}
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return self.numcantons;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSDictionary *dic = nil;
    if (tableView == _cantonTableView) {
        dic = cantons_value;
    } else if (tableView == _trainTableView) {
        dic = trains_value;
    } else {
        NSLog(@"hu?");
        return nil;
    }
    NSString *cn = [tableColumn identifier];
    if (![cn length]) return @"-";
    if ([cn isEqual:@"canton_num"]) return @(row);
    if ([cn isEqual:@"num"]) return @(row);
    if ([cn isEqual:@"train_num"]) return @(row);
    NSString *sr = [NSString stringWithFormat:@"%d", (int) row];
    NSString *k = [cn stringByReplacingOccurrencesOfString:@"#" withString:sr];
    //NSString *k = [NSString stringWithFormat:@"C%d_%@", (int)row, cn];
    NSNumber *n = [dic objectForKey:k];
    if (dic && !n) {
        if ((0)) NSLog(@"unknown %@",k);
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

#if 0
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
#endif

#pragma mark - simu

static AppDelegate *theDelegate = nil;

- (void) startSimu
{
    self.simTrain0 = [[SimTrain alloc]init];
    lastSimu = [NSDate timeIntervalSinceReferenceDate];
    t0 = lastSimu;
    simuTimer = [NSTimer timerWithTimeInterval:0.01 target:self selector:@selector(simuTimer) userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop]addTimer:simuTimer forMode:NSDefaultRunLoopMode];

    _runMode = -1;
    [self setRunMode:0];
    
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

uint32_t SimuTick = 0;
- (void) simuTimer
{
    static NSTimeInterval t0;
    static dispatch_once_t onceToken=0;
    dispatch_once(&onceToken, ^{
        t0 = [NSDate timeIntervalSinceReferenceDate];
    });
#if 0
    // "real" time
    NSTimeInterval t = [NSDate timeIntervalSinceReferenceDate]-t0;
    NSTimeInterval dt = t-lastSimu;
    lastSimu = t;
    uint32_t mdt = (uint32_t)(dt * 1000.0);
    uint32_t mt = (uint32_t)(t * 1000.0);
    SimuTick = mt;
#else
    SimuTick += 10;
    uint32_t mdt = 10;
    uint32_t mt = SimuTick;
#endif
   

    
    for (int i =0; i<2; i++) {
#if NEW_ADC_AVG
        memset((void*)&(adc_result[i]), 0, sizeof(adc_result));
#else
        memset((void*)&(train_adc_buf[i]), 0, sizeof(adc_buf_t));
#endif

    }
    
    [_simTrain0 computeTrainsAfter:mdt sinceStart:mt];
    for (int nc = 0; nc < NUM_CANTONS; nc++) {
        double bemf = [_simTrain0 bemfForCantonNum:nc];
        int bemfi = -(bemf/4.545) * 3.3 *4096;
#if NEW_ADC_AVG
        //adc_result[0].meas[nc].vA = (bemfi>0) ? 0 : -bemfi;
        //adc_result[0].meas[nc].vB = (bemfi>0) ? bemfi : 0;
        adc_result[0].meas[nc].vBA = bemfi;
#else
        train_adc_buf[0].off[nc].vA = (bemfi>0) ? 0 : -bemfi;
        train_adc_buf[0].off[nc].vB = (bemfi>0) ? bemfi : 0;
#endif
    }
    

    int notif = NOTIF_NEW_ADC_1;
   
    bemf_tick(notif,        mt, mdt);
    msgsrv_tick(notif,      mt, mdt);
    OAM_Tasklet(notif, mt, mdt);
    spdctl_run_tick(notif,  mt, mdt);
    //msgsrv_tick(notif,    mt, mdt);
    canton_tick(notif,      mt, mdt);
    turnout_tick(notif,     mt, mdt);
    //usbPollQueues();
    ctrl_run_tick(notif,    mt, mdt);

    uitrack_run_tick(notif, mt, mdt);
}

- (void) usbTimer
{
    static NSTimeInterval t0;
    static dispatch_once_t onceToken=0;
    dispatch_once(&onceToken, ^{
        t0 = [NSDate timeIntervalSinceReferenceDate];
    });
    NSTimeInterval t = [NSDate timeIntervalSinceReferenceDate]-t0;
    NSTimeInterval dt = t-lastSimu;
    lastSimu = t;
    uint32_t mdt = (uint32_t)(dt * 1000.0);
    uint32_t mt = (uint32_t)(t * 1000.0);
    SimuTick = mt;

    
    int notif = NOTIF_NEW_ADC_1;
   
    
    //msgsrv_tick(notif,      mt, mdt);
   
    uitrack_run_tick(notif, mt, mdt);
}


void train_simu_canton_volt(int numcanton, int voltidx, int vlt100)
{
    [theDelegate simuSetVoltCanton:numcanton voltidx:voltidx vlt100:vlt100];
}

- (void) simuSetVoltCanton:(int)numcanton voltidx:(int)voltidx vlt100:(int)vlt100
{
    double vlt = vlt100/100.0;
    // TODO addr to num canton
    [_simTrain0 setVolt:vlt forCantonNum:numcanton];
#if 0
    if (numcanton == [_simTrain0 simuCurCanton]) {
        [_simTrain0 setVolt:vlt];
    }
    switch (numcanton) {
        case 0:
            self.C0_vidx = voltidx;
            //self.canton_0_centivolts = vlt;
            [_simTrain0 setVolt:vlt];
            break;
        case 1:
            //self.canton_1_centivolts = vlt;
            self.C1_vidx = voltidx;
            break;
        default:
            break;
    }
#endif
}

void train_simu_canton_set_pwm(int numcanton, int8_t dir, int duty)
{
    [theDelegate simuSetPwmCanton:numcanton dir:dir duty:duty];
}

- (void) simuSetPwmCanton:(int)numcanton dir:(int)dir duty:(int)duty
{
    //int sduty = duty*dir;
    [_simTrain0 setPwm:duty dir:dir forCantonNum:numcanton];
#if 0
    if (numcanton == [_simTrain0 simuCurCanton]) {
        [_simTrain0 setPwm:duty dir:dir];
    }
    switch (numcanton) {
        case 0:
            self.C0_pwm = sduty;
            break;
        case 1:
            self.C1_pwm = sduty;
            break;
        default:
            break;
    }
#endif
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

/*
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
 */
#endif

int cur_freqhz = 50;
int tsktick_freqhz = 50;
void set_pwm_freq(int freqhz, int crit)
{
}
int get_pwm_freq(void)
{
    return cur_freqhz;
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
    NSFileHandle *r;
    NSURL *fu = [self createTempCsvFile:@"record" second:nil retfile:&r];
    recordFile = r;
    if (!recordFile) {
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
    recordFile = nil;
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
            /*NSLog(@"out %@\n%@", [self dumpFrames:d],
                  [[NSString alloc]initWithData:d encoding:NSUTF8StringEncoding]);*/
        }

    };
    [gnuplotTask launchAndReturnError:&err];
    if (err) {
        NSLog(@"launch error : %@\n", err);
        gnuplotTask = nil;
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

- (NSString *) _buildGnuplotCurves:(NSArray *)col x:(int)numx file:(NSString *)file items:(NSDictionary *)recordItems
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

- (NSString *) buildGnuplotCmd:(NSArray *)col x:(NSString *)colx file:(NSString *)file items:(NSDictionary *)recordItems
{
    int numx = 0;
    NSString *namex = @"t";
    if (colx) {
        numx = [[recordItems objectForKey:colx]intValue];
        namex = colx;
    }
    NSMutableString *res = [[NSMutableString alloc]initWithCapacity:400];
    [res appendFormat:@"set terminal aqua\r\nset xlabel \"%@\"\r\n", namex];
    [res appendString:[self _buildGnuplotCurves:col x:numx file:file items:recordItems]];
    [res appendString:@"\r\n"];
    NSLog(@"gnuplot command :\n%@\n", res);
    return res;
}

- (void) gnuplot:(NSArray *)col x:(NSString *)colx file:(NSString *)file items:(NSDictionary *)recordItems
{
    NSString *cmd = [self buildGnuplotCmd:col x:colx file:file items:recordItems];
    [self plotWithGnuplot:cmd];
    
}



- (void) gnuplotGraph:(int)ngraph
{
    static NSDictionary *graphs = nil;
    if (!graphs) graphs =
    @{ @"power0"  : @[ @"tick", @"T0_ctrl_target_speed", @"T0_spd_curspeed",
                      @"C0_pwm", @"C0_vidx", @"C1_pwm", @"C1_vidx", @"C2_pwm", @"C2_vidx"],
       //@"power2" : @[ @"spd_curspeed", /*@"canton_0_centivolts",*/ @"C0_pwm", @"C0_vidx"],
       @"BEMF_T0"   : @[ @"tick",  @"T0_pid_target", @"T0_spd_curspeed", @"T0_bemf_mv" /*,  @"T0_pid_sum_e"*/],
       @"BEMF_T1"   : @[ @"tick",  @"T1_pid_target", @"T1_spd_curspeed", @"T1_bemf_mv" /*,  @"T0_pid_sum_e"*/],
       @"Vsense" : @[ @"tick",  @"T0_bemf_mv"],

       @"PID_T0"    : @[ @"tick", @"T0_pid_target",@"T0_pid_target_v", @"T0_bemf_mv", @"T0_pid_last_err", @"T0_pid_sum_e"],
       @"PID_T1"    : @[ @"tick", @"T1_pid_target",@"T1_pid_target_v", @"T1_bemf_mv", @"T1_pid_last_err", @"T1_pid_sum_e"],

       @"inertia": @[@"tick", @"T0_ine_t", @"T0_ine_c", @"T1_ine_t", @"T1_ine_c"],
       @"pose"   : @[@"tick", @"T0_spd_curspeed", @"T0_bemf_mv", @"T0_ctrl_dir", @"T0_pose", @"T0_pose_trig1",@"T0_pose_trig2", @"T0_curposmm", @"T0_beginposmm"],
       @"pose2"   : @[@"tick", @"T0_spd_curspeed", @"T0_bemf_mv", @"T0_ctrl_dir", @"T0_curposmm", @"T0_beginposmm"],
       @"INA3221"   : @[ @"tick", @"ina0", @"ina1", @"ina2", @"T0_bemf_mv" ],
    };
    NSString *k = nil;
    switch (ngraph) {
        case 0: k=@"power0"; break;
        case 1: k=@"Vsense"; break;
        case 2: k=@"BEMF_T0"; break;
        case 3: k=@"BEMF_T1"; break;
        case 4: k=@"PID_T0"; break;
        case 5: k=@"PID_T1"; break;
        case 6: k=@"inertia"; break;
        case 7: k=@"pose"; break;
        case 8: k=@"pose2"; break;
        case 9: k=@"INA3221"; break;
        default:
            return;
    }
    NSArray *t = [graphs objectForKey:k];
    if (!t) return;
    NSString *colx = [t objectAtIndex:0];
    if (![colx length]) colx = nil;
    NSString *f = [self.fileURL path];
    [self gnuplot:[t subarrayWithRange:NSMakeRange(1, [t count]-1)] x:colx file:f items:recordItems];
    
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

#pragma mark -

- (void) sendMsg64:(msg_64_t)m
{
    if (_linkok<LINK_FRM) return;
    if (self.linkok == LINK_SIMULOW) {
        mqf_write_from_usb(&m);
        return;
    }
    uint8_t buf9[9];
    buf9[0] = FRAME_M64;
    memcpy(buf9+1, &m, 8);
    
    int fd = [usb fileDescriptor];
    write(fd, buf9, 9);
}

#pragma mark - canton test

- (IBAction) btnRunMode:(id)sender
{
    int tag = (int) [sender tag];
    /*
     ATTENTION : tag (and paremeter given to setRunMode: are NOT run_mode values
     (see below)
     */
    _runMode = -1;
    [self setRunMode:tag];
}
- (void) setRunMode:(NSUInteger)testMode
{
    if (testMode == _runMode) return;
    _runMode = testMode;
    
   
    msg_64_t m;
    m.to = MA3_BROADCAST;
    m.from = MA3_UI_GEN; //(UISUB_USB);
    m.cmd = CMD_SETRUN_MODE;
    switch (testMode) {
        case 0: m.v1u = runmode_normal; break;
        case 1: m.v1u = runmode_testcanton; break;
        case 2: m.v1u = runmode_testcanton; break;
        case 3: m.v1u = runmode_detect_experiment; break;
        case 4: m.v1u = runmode_detect2; break;
        case 5: m.v1u = runmode_off; break;
        case 6: m.v1u = runmode_testcan; break;
        default:
            break;
    }
    [self sendMsg64:m];
}

- (void) setTestCanton:(NSUInteger)testCanton
{
    if (_runMode != 1) return;
    if (testCanton == _testCanton) return;
    NSUInteger oldCanton = _testCanton;
    _testCanton = testCanton;
    if (_runMode == 1) {
        for (int i = 0; i<NUM_CANTONS; i++) {
            if (i==oldCanton) {
                msg_64_t m;
                xblkaddr_t bi = {.v = i};
                TO_CANTON(m, bi);
                m.from = MA3_UI_GEN; //(UISUB_USB);
                m.cmd = CMD_BEMF_OFF;
                [self sendMsg64:m];
                continue;
            }
            if (i!=testCanton) continue;
            msg_64_t m;
            xblkaddr_t bi = {.v = i};
            TO_CANTON(m, bi);
            m.from = MA3_UI_GEN; //(UISUB_USB);
            m.cmd = CMD_BEMF_ON;
            [self sendMsg64:m];
            m.cmd = CMD_SETVPWM;
            m.v1u = 7;
            m.v2 = 0;
            [self sendMsg64:m];
        }
    }
    NSInteger p = _testPWM;
    _testPWM = -9999;
    self.testPWM = p;
}
- (void) setTestVoltIdx:(NSUInteger)testVoltIdx
{
    if (!_runMode) return;
    if (testVoltIdx == _testVoltIdx) return;
    _testVoltIdx = testVoltIdx;
    [self sendVPWM];
}

- (void) setTestPWM:(NSInteger)testPWM
{
    if (!_runMode) return;
    if (testPWM == _testPWM) return;
    _testPWM = testPWM;
    [self sendVPWM];
}
- (void) sendVPWM
{
    msg_64_t m = {0};
    if (_runMode == 1) {
        xblkaddr_t tc = {.v = _testCanton};
        TO_CANTON(m, tc);
    } else {
        m.to = MA3_BROADCAST;
    }
    //m.to = (_runMode == 1) ? MA_CANTON(0, _testCanton) : MA_BROADCAST;
    m.from = MA3_UI_GEN; //(UISUB_USB);
    m.cmd = CMD_SETVPWM;
    m.v1u = _testVoltIdx;
    m.v2 = _testPWM;
    [self sendMsg64:m];
}

#pragma mark -

- (IBAction) triggerOscillo:(id)sender
{
    msg_64_t m;
    m.to = MA1_SPDCTL(0);
    m.from = MA3_UI_GEN; //(UISUB_USB);
    m.cmd = CMD_TRIG_OSCILLO;
    m.v1u = 0;
    m.v2 = 9;
    [self sendMsg64:m];
}


#pragma mark -



void impl_uitrack_change_blk(int blk, int v, int trn, int sblk)
{
    [theDelegate.ctcManager uitrac_change_blk:blk val:v train:trn sblk:sblk];
}
void impl_uitrack_change_tn(int tn, int v)
{
    [theDelegate.ctcManager uitrac_change_tn:tn val:v];
}

void impl_uitrack_change_tn_reserv(int tn, int train)
{
    [theDelegate.ctcManager uitrac_change_tn_reser:tn train:train];
}




@end

void FatalError(const char *shortsmsg, const char *longmsg, enum fatal_error_code errcode)
{
    abort();
}
void Error_Handler(void)
{
    //NSAssert(0, "Error handler");
    abort();
}
