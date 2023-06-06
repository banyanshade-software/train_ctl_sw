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
#include "longtrain4.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"

@interface TestLongTrainLeft0 : XCTestCase

@end

@implementation TestLongTrainLeft0 {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


static lsblk_num_t s21 = {21};
static lsblk_num_t s14 = {14};


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
    ctrl3_init_train(0, &tvars, s21, 0, 1);
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
    tvars.beginposmm = beg;
    tvars._curposmm = 600+beg;
    
    /*   b18  b19  b20     b22
        | 20 | 20 | 30  | 700           |....
                              xxxxL(100)
                              150
        | 20 + 20 + 30  +                       (70)
                        +  (210)                brake at 210+150 = 360
                        +  (50)                 stop at 50+150 = 200
     */
    //ctrl3_get_next_sblks(0, &tvars, tconf);
    //XCTAssert(tvars.leftcars.numlsblk == 0);
    //XCTAssert(tvars.leftcars.rlen_mm == 10*(70-10-15)); //45
   
    rettrigs_t rettrigs = {0};
    //rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    rc = _lt4_get_trigs(0, &tvars, tconf, 1, &rettrigs, 0);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{150+beg, tag_chkocc}, {200+beg,tag_stop_eot}, {360+beg, tag_brake}, {0+beg,tag_end_lsblk}, {0,0}}};
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
    tvars.beginposmm = beg;
    tvars._curposmm = cp*10+beg;
    
    //ctrl3_get_next_sblks(0, &tvars, tconf);
    //XCTAssert(tvars.leftcars.numlsblk == 0);
    //XCTAssert(tvars.leftcars.rlen_mm == 10*(cp-15)); //45
   
    rettrigs_t rettrigs = {0};
    //rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    rc = _lt4_get_trigs(0, &tvars, tconf, 1, &rettrigs, 0);
    XCTAssert(rc>0);
    XCTAssert(rc==10*(cp-15+7-12));
    const rettrigs_t expt1 = { 0, 0, 0, 0, 3, {{150+beg, tag_chkocc}, {200+beg,tag_stop_eot}, {0+beg,tag_end_lsblk}, {0,0}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}







- (void)testCheckTagBrake1
{
    [self checkTagBrake:0 d:37];
}

- (void)testCheckTagBrake2
{
    [self checkTagBrake:0 d:38];
}


- (void)testCheckTagBrake1a
{
    [self checkTagBrake:1000 d:37];
}

- (void)testCheckTagBrake2a
{
    [self checkTagBrake:1000 d:38];
}

- (void)testCheckTagBrake1b
{
    [self checkTagBrake:-1000 d:37];
}

- (void)testCheckTagBrake2b
{
    [self checkTagBrake:-1000 d:38];
}


- (void) checkTagBrake:(int)beg d:(int)cp
{
    int rc;

    tconf->trainlen_left_cm = 15;
    tconf->trainlen_right_cm = 0;
    occupency_clear();
    
    // (A)
    tvars.beginposmm = beg;
    tvars._curposmm = cp*10+beg;
    
    //ctrl3_get_next_sblks(0, &tvars, tconf);
    //XCTAssert(tvars.leftcars.numlsblk == 0);
    //XCTAssert(tvars.leftcars.rlen_mm == 10*(cp-15)); //45
   
    rettrigs_t rettrigs = {0};
    //rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    rc = _lt4_get_trigs(0, &tvars, tconf, 1, &rettrigs, 0);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{150+beg, tag_chkocc}, {200+beg,tag_stop_eot}, {360+beg,tag_brake}, {beg,tag_end_lsblk}, {0,0}}};
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
    tvars.beginposmm = beg;
    tvars._curposmm = cp*10+beg;
    
    /*ctrl3_get_next_sblks(0, &tvars, tconf);
    if (b) {
        XCTAssert(tvars.leftcars.numlsblk == 1);
        XCTAssert(tvars.leftcars.rlen_mm == 10*(70+3-cp-15));
    } else {
        XCTAssert(tvars.leftcars.numlsblk == 0);
        XCTAssert(tvars.leftcars.rlen_mm == 10*(cp-15)); //45
    }*/
    rettrigs_t rettrigs = {0};
    //rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    rc = _lt4_get_trigs(0, &tvars, tconf, 1, &rettrigs, 0);
    XCTAssert(rc<0);
    if (b) {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{150+beg, tag_chkocc}, {0+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    } else {
        const rettrigs_t expt1 = { 1, 0, 0, 0, 2, {{150+beg, tag_chkocc},  {0+beg,tag_end_lsblk}, {0,0}, {0,0}}};
        XCTAssert(!cmptrigs(&rettrigs, &expt1));
    }
}


@end

