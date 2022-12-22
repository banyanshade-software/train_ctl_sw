//
//  TestLTFsm.m
//  train_throttle
//
//  Created by Daniel Braun on 20/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlLT.h"
#include "longtrain.h"
#include "trig_tags.h"

@interface TestLTFsm : XCTestCase

@end

@implementation TestLTFsm {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


static lsblk_num_t snone = {-1};
static lsblk_num_t szero = {0};
static lsblk_num_t sone = {1};
static lsblk_num_t stwo = {2};

static const xtrnaddr_t to0 = { .v = 0};
static const xtrnaddr_t to1 = { .v = 1};

- (void)setUp
{
    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topology_set_turnout(to0, topo_tn_straight, -1);
    topology_set_turnout(to1, topo_tn_turn, -1);


    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, sone);
    NSLog(@"init done");
}

- (void)tearDown {
    extern int errorhandler;
    XCTAssert(errorhandler==0);
}




- (void) startRight
{
    XCTAssert(tvars._state == train_state_station);
    
    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    ctrl3_upcmd_set_desired_speed_zero(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    // start right
    ctrl3_upcmd_set_desired_speed(0, &tvars, 90);
    
}




- (void)testStartRightNormal {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 12;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
}

- (void)testStartRightEot {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 90;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_station);
    XCTAssert(tvars._sdir == 0);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
}

- (void)testStartRightBrake1 {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 80;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 40);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
}

- (void)testStartRightBrake2 {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 78;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 60);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
}

- (void)testStartRightOcc {
    topology_set_turnout(to1, topo_tn_straight, -1);
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 40;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_blkwait);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 90);
}


- (void)testStartLeft {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 19;
    tvars._curposmm = 30;
    
    XCTAssert(tvars._state == train_state_station);
    
    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    ctrl3_upcmd_set_desired_speed_zero(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    // start right
    ctrl3_upcmd_set_desired_speed(0, &tvars, -90);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == -1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == -90);
    XCTAssert(tvars._spd_limit == 99);
}


- (void) testStop
{
    [self testStartRightNormal];
    ctrl3_upcmd_set_desired_speed_zero(0, &tvars);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);
    XCTAssert(tvars._sdir == 0);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
}

- (void) testChangeDir
{
    [self testStartRightNormal];
    ctrl3_upcmd_set_desired_speed(0, &tvars, -90);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);
    XCTAssert(tvars._sdir == 0);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
}


- (void)testStartRightOccThenOk
{
    topology_set_turnout(to1, topo_tn_straight, -1);
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 40;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_blkwait);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 90);
    
    // train is blk wait on turn out 1, change turnout 1
    // and it should start
    topology_set_turnout(to1, topo_tn_turn, -1);
    ctrl3_occupency_updated(0, &tvars);
    
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._target_unisgned_speed == 90);
    
}

- (void)testStartRightOccThenOk2
{
    [self testStartRightOccThenOk];
    
    // while running, an occupency update without
    // any change for this train should not affect it
    
    ctrl3_occupency_updated(0, &tvars);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._target_unisgned_speed == 90);
    
}




- (void) startLeft
{
    XCTAssert(tvars._state == train_state_station);
    
    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    ctrl3_upcmd_set_desired_speed_zero(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    // start right
    ctrl3_upcmd_set_desired_speed(0, &tvars, -90);
    
}




- (void)testStartLeftNormal {
    tconf->trainlen_left_cm = 12;
    tconf->trainlen_right_cm = 0;
    tvars._curposmm = 30;
    [self startLeft];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == -1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == -90);
    XCTAssert(tvars._spd_limit == 99);
}

@end
