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
    tvars.beginposmm = beg*10;
    tvars._curposmm = 100+beg*10;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_cm == 70-10-15); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{55+beg, tag_chkocc}, {50+beg,tag_stop_eot}, {34+beg, tag_brake}, {70+beg,tag_end_lsblk}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}




- (void)testCheckBrake1
{
    [self checkBrake:0 d:40];
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
    tvars.beginposmm = beg*10;
    tvars._curposmm = cp*10+beg*10;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_cm == 70-cp-15); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==16-(cp-49+15));
    const rettrigs_t expt1 = { 0, 0, 0, 0, 3, {{55+beg, tag_chkocc}, {50+beg,tag_stop_eot}, {70+beg,tag_end_lsblk}, {0,0}, {0,0}}};
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
    tvars.beginposmm = beg*10;
    tvars._curposmm = cp*10+beg*10;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    if (b) {
        XCTAssert(tvars.rightcars.numlsblk == 1);
        XCTAssert(tvars.rightcars.rlen_cm == 70+3-cp-15); //45
    } else {
        XCTAssert(tvars.rightcars.numlsblk == 0);
        XCTAssert(tvars.rightcars.rlen_cm == 70-cp-15); //45
    }
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc<0);
    if (b) {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{58+beg, tag_chkocc},  {70+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    } else {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{55+beg, tag_chkocc},  {70+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    }
}

/*
 15 010390      state=running   dir=1 sblk=4 spd= 74 dspd= 74 pos=176 from 125
 16 010830 TRIG end_lsblk pos=342
 17 010830 --set chkocc    pos=630
 18 010830 --set stop_eot  pos=510
 19 010830 --set brake     pos=350
 20 010830      state=running   dir=1 sblk=5 spd= 74 dspd= 74 pos=342 from 342
 21 010840 TRIG end_lsblk pos=352
 22 010840      state=running   dir=1 sblk=5 spd= 74 dspd= 74 pos=352 from 342
 23 010850 TRIG brake     pos=350
 24 010850 --set chkocc    pos=640
 25 010850 --set stop_eot  pos=520
 26 010850 --set brake     pos=360 <------- should not be trigged
 27 010850      state=running   dir=1 sblk=5 spd= 74 dspd= 74 pos=350 from 342
 */

- (void) testBrake350
{
    int rc;
    
    ctrl3_init_train(0, &tvars, s5, 1);

    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 15;
    occupency_clear();
    
    rettrigs_t rettrigs = {0};

    tvars.beginposmm = 342;
    tvars._curposmm = 350;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc>0);
}
@end

