//
//  TestLongTrainRight2.m
//  trainctlTests
//
//  Created by Daniel Braun on 02/04/2023.
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

@interface TestLongTrainRight2 : XCTestCase

@end

@implementation TestLongTrainRight2 {
    
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


static const lsblk_num_t s26 = {26};
static const xtrnaddr_t to3 = { .v = 3};
static const xblkaddr_t c0 = {.v = 0};
static const xblkaddr_t c1 = {.v = 1};
static const xblkaddr_t c2 = {.v = 2};


- (void)setUp {
    errorhandler = 0;

    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    topology_set_turnout(to3, topo_tn_straight, -1);
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    

    tvars._mode = train_manual;
    
    tconf->trainlen_left_cm = 13;
    tconf->trainlen_right_cm = 17;
    
    ctrl3_init_train(0, &tvars, s26, 0, 1);
    
    tvars.beginposmm = 0;
    tvars._curposmm = 50;
    tvars._sdir = 1;
    NSLog(@"init done");
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
    
}


- (void) testRightNoBack1
{
    /*
          25 c0        26 c1         27 c2
     |----------xxx|xLxxx      |           ||
              (130)  (170)    /
                     ^50
     
     brk:
                    |          |      (280)||
                            Lxx xxxx
                              50 120   --> brake at 350
     */
    int rc;
    rettrigs_t rettrigs = {0};
    rc = _lt4_get_trigs(0, &tvars, tconf, 0, &rettrigs, 0);
    //rc = lt4_get_trigs(0, &tvars, tconf, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 1, 0, 3, { {200, tag_need_c2}, {230,tag_chkocc}, {350,tag_brake}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
    int occ25 = get_block_addr_occupency(c0);
    int occ26 = get_block_addr_occupency(c1);
    int occ27 = get_block_addr_occupency(c2);
    XCTAssert(BLK_OCC_CARS==occ25);
    XCTAssert(BLK_OCC_LOCO_RIGHT==occ26);
    XCTAssert(BLK_OCC_FREE==occ27);
}


- (void) testRightBack1
{
    int rc;
    // freeback at 130+margin(100) = 230
    rettrigs_t rettrigs = {0};
    rc = lt4_get_trigs(0, &tvars, tconf, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 1, 0, 4, { {230, tag_free_back}, {200, tag_need_c2}, {230,tag_chkocc}, {350,tag_brake}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}
@end
