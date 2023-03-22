//
//  TestLongTrainLeft1.m
//  trainctlTests
//
//  Created by Daniel Braun on 21/03/2023.
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

@interface TestLongTrainLeft1 : XCTestCase

@end

@implementation TestLongTrainLeft1{
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
    ctrl3_init_train(0, &tvars, s14, 0, 1);
    NSLog(@"init done");
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
    
}



- (void) testCheckBrakeC14
{;
    [self checkBrakec14:-700 posmm:-40];
}
- (void) checkBrakec14:(int)beg posmm:(int)pmm
{
    int rc;
    
    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, s14, pmm, 1);
    tconf->trainlen_left_cm = 15;
    tconf->trainlen_right_cm = 0;
    occupency_clear();
    
    // |--|--------------xxxxxx--..
    //  30  -700         <150>-30
    tvars.beginposmm = beg;
    tvars._curposmm = pmm;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.leftcars.numlsblk == 0);
    XCTAssert(tvars.leftcars.rlen_mm == 700+pmm-150); //45
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    //XCTAssert(rc==10*(cp-15+7-12));
    const rettrigs_t expt1 = { 0, 0, 0, 0, 4, {{150+beg, tag_chkocc}, {-30+120+150+beg,tag_stop_eot}, {-30+120+150+160+beg, tag_brake}, {beg,tag_end_lsblk}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}


@end
