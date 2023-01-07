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

@interface TestLongTrain0 : XCTestCase

@end

@implementation TestLongTrain0 {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}

int cmptrigs(const rettrigs_t *r1, const rettrigs_t *r2);

static lsblk_num_t s13 = {13};
static lsblk_num_t s14 = {14};

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
    const rettrigs_t expt1 = { 0, 0, 3, {{55+beg, tag_chkocc}, {50+beg,tag_stop_eot}, {34+beg, tag_brake}, {0,0}, {0,0}}};
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
    XCTAssert(rc==cp-49+15);
    const rettrigs_t expt1 = { 0, 0, 2, {{55+beg, tag_chkocc}, {50+beg,tag_stop_eot}, {0,0}, {0,0}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}
@end


int cmptrigs(const rettrigs_t *r1, const rettrigs_t *r2)
{
    if (r1->isocc != r2->isocc) return -1;
    if (r1->isoet != r2->isoet) return -2;
    for (int i=0; i<NUMTRIGS; i++) {
        if (r1->trigs[i].poscm != r2->trigs[i].poscm) {
            return i+10;
        }
        if (r1->trigs[i].tag != r2->trigs[i].tag) {
            return i+100;
        }
    }
    return 0;
}
