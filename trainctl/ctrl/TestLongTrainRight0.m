//
//  TestLongTrain0.m
//  trainLPctlTests
//
//  Created by Daniel Braun on 04/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlLT.h"
#include "longtrain.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"

@interface TestLongTrainRight0 : XCTestCase

@end

@implementation TestLongTrainRight0 {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


//static lsblk_num_t s13 = {13};
static lsblk_num_t s14 = {14};
static lsblk_num_t s5 = {5};
static lsblk_num_t s4 = {4};


- (void)setUp {
    
    errorhandler = 0;

    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    

    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, s14, 1);
    NSLog(@"init done");
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
}




- (void) testChkRight
{
    [self chkRight:0];
}
- (void) testChkRight2
{
    [self chkRight:1000];
}
- (void) testChkRight2b
{
    [self chkRight:123];
}
- (void) testChkRight3
{
    [self chkRight:-1000];
}
- (void) chkRight:(int)beg
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 15;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg;
    tvars._curposmm = 100+beg;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(70-10-15)); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{550+beg, tag_chkocc}, {500+beg,tag_stop_eot}, {340+beg, tag_brake}, {700+beg,tag_end_lsblk}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}




- (void)testCheckBrake1
{
    [self checkBrake:0 d:40];
}


- (void)testCheckBrake34
{
    [self checkBrake:0 d:34];
}
- (void)testCheckBrake2
{
    [self checkBrake:0 d:41];
}

- (void)testCheckBrake3
{
    [self checkBrake:0 d:49];
}
- (void)testCheckBrake4
{
    [self checkBrake:0 d:48];
}

- (void)testCheckBrake1b
{
    [self checkBrake:1000 d:40]; // 6
}

- (void)testCheckBrake2b
{
    [self checkBrake:1000 d:47];
}

- (void)testCheckBrake1c
{
    [self checkBrake:-1000 d:40];
}

- (void)testCheckBrake2c
{
    [self checkBrake:-1000 d:47];
}
- (void) checkBrake:(int)beg d:(int)cp
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 15;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg;
    tvars._curposmm = cp*10+beg;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(70-cp-15)); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==10*(16-(cp-49+15)));
    const rettrigs_t expt1 = { 0, 0, 0, 0, 3, {{550+beg, tag_chkocc}, {500+beg,tag_stop_eot}, {700+beg,tag_end_lsblk}, {0,0}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}


- (void)testCheckTagBrake1
{
    [self checkTagBrake:0 d:33];
}
- (void)testCheckTagBrake2
{
    [self checkTagBrake:0 d:30];
}

- (void)testCheckTagBrake1a
{
    [self checkTagBrake:1000 d:33];
}
- (void)testCheckTagBrake2a
{
    [self checkTagBrake:1000 d:30];
}

- (void)testCheckTagBrake1b
{
    [self checkTagBrake:-1000 d:33];
}
- (void)testCheckTagBrake2b
{
    [self checkTagBrake:-1000 d:30];
}

- (void) checkTagBrake:(int)beg d:(int)cp
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 15;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg;
    tvars._curposmm = cp*10+beg;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(70-cp-15)); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{550+beg, tag_chkocc}, {500+beg,tag_stop_eot}, {340+beg, tag_brake}, {700+beg,tag_end_lsblk}, {0,0}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}


- (void)testCheckStop1
{
    [self checkStop:0 d:51 b:0];
}

- (void)testCheckStop2
{
    [self checkStop:0 d:55 b:1];
}

- (void)testCheckStop3
{
    [self checkStop:1000 d:56 b:1];
}

- (void) checkStop:(int)beg d:(int)cp b:(int)b
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 15;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg;
    tvars._curposmm = cp*10+beg;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    if (b) {
        XCTAssert(tvars.rightcars.numlsblk == 1);
        XCTAssert(tvars.rightcars.rlen_mm == 10*(70+3-cp-15)); //45
    } else {
        XCTAssert(tvars.rightcars.numlsblk == 0);
        XCTAssert(tvars.rightcars.rlen_mm == 10*(70-cp-15)); //45
    }
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc<0);
    if (b) {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{580+beg, tag_chkocc},  {700+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    } else {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{550+beg, tag_chkocc},  {700+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    }
}

#if 0
/*
 0 000010      state=station   dir=0 sblk=2 spd=  0 dspd=  0 pos=9999999 from 0
 1 005970 --set  end_lsblk pos=600
 2 005970      state=running   dir=1 sblk=2 spd= 81 dspd= 81 pos=9999999 from 0
 3 007090 --set  end_lsblk pos=120
 4 007090      state=running   dir=1 sblk=1 spd= 81 dspd= 81 pos=9999999 from 0
 5 007290 TRIG   end_lsblk pos=9999999->127
 6 007290 --set  chkocc    pos=160
 7 007290 --set  end_lsblk pos=340
 8 007290      state=running   dir=1 sblk=4 spd= 81 dspd= 81 pos=127 from 127
 9 007370 TRIG   chkocc    pos=127->160
10 007370 --set  chkocc    pos=170
11 007370 --set  end_lsblk pos=340
12 007370      state=running   dir=1 sblk=4 spd= 81 dspd= 81 pos=160 from 127
13 007410 TRIG   chkocc    pos=160->176
14 007410 --set  end_lsblk pos=340
15 007410      state=running   dir=1 sblk=4 spd= 81 dspd= 81 pos=176 from 127
16 007810 TRIG   end_lsblk pos=176->342
17 007810 --set  chkocc    pos=630
18 007810 --set  stop_eot  pos=510
19 007810 --set  brake     pos=350
20 007810      state=running   dir=1 sblk=5 spd= 81 dspd= 81 pos=342 from 342 xxxx
21 007830 TRIG   brake     pos=342->350
22 007830 --set  chkocc    pos=640
23 007830 --set  stop_eot  pos=520
24 007830 --set  brake     pos=360
25 007830      state=running   dir=1 sblk=5 spd= 81 dspd= 81 pos=350 from 342
26 007860 TRIG   brake     pos=350->367
 
 s4 len = 22
 s5 len = 47
 47-18-12-16 = 1 (brake)
 eot at 17
 */

- (void) testBrake350
{
    int rc;
    
    ctrl3_init_train(0, &tvars, s5, 1);

    tconf->trainlen_left_cm = 6;
    tconf->trainlen_right_cm = 18;
    occupency_clear();
    
    rettrigs_t rettrigs = {0};

    tvars.beginposmm = 342;
    tvars._curposmm = 350;
    tvars._state = train_state_running;
    tvars._sdir = 1;
    tvars._desired_signed_speed = 81;
    tvars._spd_limit = 99;
    tvars._target_unisgned_speed = 81;
    tvars.can1_xaddr.v = 2;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc>0);
}

#endif
@end

