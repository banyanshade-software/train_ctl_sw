//
//  TestLongTrainLeft0.m
//  trainLPctlTests
//
//  Created by Daniel Braun on 04/01/2023.
//  CopyLeft © 2023 Daniel BRAUN. All Lefts reserved.
//

#import <XCTest/XCTest.h>

#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlLT.h"
#include "longtrain.h"
#include "trig_tags.h"

@interface TestLongTrainLeft0 : XCTestCase

@end

@implementation TestLongTrainLeft0 {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}

int cmptrigs(const rettrigs_t *r1, const rettrigs_t *r2);

static lsblk_num_t s21 = {21};

extern int errorhandler;

- (void)setUp {
    
    errorhandler = 0;
    //extern uint8_t topology_num;
    //topology_num = 0;

    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    

    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, s21, 1);
    NSLog(@"init done");
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
}




- (void) testChkLeft
{
    [self chkLeft:0];
}
- (void) testChkLeft2
{
    [self chkLeft:1000];
}
- (void) testChkLeft2b
{
    [self chkLeft:123];
}
- (void) testChkLeft3
{
    [self chkLeft:-1000];
}
- (void) chkLeft:(int)beg
{
    int rc;
    
    tconf->trainlen_left_cm = 15;
    tconf->trainlen_right_cm = 0;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg*10;
    tvars._curposmm = 600+beg*10;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.leftcars.numlsblk == 0);
    XCTAssert(tvars.leftcars.rlen_cm == 70-10-15); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{15+beg, tag_chkocc}, {20+beg,tag_stop_eot}, {36+beg, tag_brake}, {0+beg,tag_end_lsblk}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
    
    
}

- (void)testCheckBrake1
{
    [self checkBrake:0 d:36];
}

- (void)testCheckBrake2
{
    [self checkBrake:0 d:30];
}

- (void)testCheckBrake3
{
    [self checkBrake:0 d:27];
}


- (void)testCheckBrake1b
{
    [self checkBrake:1000 d:28];
}

- (void)testCheckBrake2b
{
    [self checkBrake:-1000 d:28];
}



- (void) checkBrake:(int)beg d:(int)cp
{
    int rc;

    tconf->trainlen_left_cm = 15;
    tconf->trainlen_right_cm = 0;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg*10;
    tvars._curposmm = cp*10+beg*10;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.leftcars.numlsblk == 0);
    XCTAssert(tvars.leftcars.rlen_cm == cp-15); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==cp-15+7-12);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 3, {{15+beg, tag_chkocc}, {20+beg,tag_stop_eot}, {0+beg,tag_end_lsblk}, {0,0}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}



- (void)testCheckStop1
{
    [self checkStop:0 d:20 b:0];
}

- (void)testCheckStop2
{
    [self checkStop:0 d:19 b:0];
}

- (void)testCheckStop3
{
    [self checkStop:1000 d:20 b:0];
}
- (void)testCheckStop4
{
    [self checkStop:-1000 d:20 b:0];
}


- (void) checkStop:(int)beg d:(int)cp b:(int)b
{
    int rc;

    tconf->trainlen_right_cm = 0;
    tconf->trainlen_left_cm = 15;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg*10;
    tvars._curposmm = cp*10+beg*10;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    if (b) {
        XCTAssert(tvars.leftcars.numlsblk == 1);
        XCTAssert(tvars.leftcars.rlen_cm == 70+3-cp-15);
    } else {
        XCTAssert(tvars.leftcars.numlsblk == 0);
        XCTAssert(tvars.leftcars.rlen_cm == cp-15); //45
    }
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc<0);
    if (b) {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{15+beg, tag_chkocc}, {0+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    } else {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{15+beg, tag_chkocc},  {0+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    }
}


@end

