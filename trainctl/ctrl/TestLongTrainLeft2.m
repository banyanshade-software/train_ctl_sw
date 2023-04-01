//
//  TestLongTrainLeft2.m
//  trainLPctlTests
//
//  Created by Daniel Braun on 22/03/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlLT.h"
#include "longtrain4.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"

@interface TestLongTrainLeft2 : XCTestCase

@end

@implementation TestLongTrainLeft2 {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


static const lsblk_num_t s27 = {27};

static const xtrnaddr_t to3 = { .v = 3};


- (void)setUp {
    
    errorhandler = 0;
    //extern uint8_t topology_num;
    //topology_num = 0;

    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    topology_set_turnout(to3, topo_tn_straight, -1);
    //topology_set_turnout(to3, topo_tn_turn, -1);
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    

    tvars._mode = train_manual;
    
    tconf->trainlen_left_cm = 15;
    tconf->trainlen_right_cm = 1;
    
    ctrl3_init_train(0, &tvars, s27, 0, 1);
    
    tvars.beginposmm = -400;
    tvars._curposmm = -50;
    
    NSLog(@"init done");
    
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
    
}



- (void) testLeft2
{
    int rc;
    
    //ctrl3_get_next_sblks(0, &tvars, tconf);
    //XCTAssert(tvars.leftcars.numlsblk == 0);
    //XCTAssert(tvars.leftcars.rlen_mm == 400-50-150);
   
    rettrigs_t rettrigs = {0};
    //rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    rc = lt4_get_trigs(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    //XCTAssert(rc==10*(cp-15+7-12));
    const rettrigs_t expt1 = { 0, 0, 0, 0, 2, {{-400+150, tag_chkocc}, {-400,tag_end_lsblk}, {0,0}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}


@end
