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
#include "longtrain4.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"

@interface TestLTFsm : XCTestCase

@end




@implementation TestLTFsm {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


//static lsblk_num_t snone = {-1};
//static lsblk_num_t szero = {0};
static lsblk_num_t sone = {1};
//static lsblk_num_t stwo = {2};

static const xtrnaddr_t to0 = { .v = 0};
static const xtrnaddr_t to1 = { .v = 1};

- (void)setUp
{
    memset(&tvars, 0, sizeof(tvars));
    mqf_clear(&from_ctrl);
    errorhandler = 0;
    tconf = (conf_train_t *) conf_train_get(0);
    // some tests change this, so reset it to "normal" values
    tconf->trainlen_left_cm = 6;
    tconf->trainlen_right_cm = 12;
    
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topology_set_turnout(to0, topo_tn_straight, -1);
    topology_set_turnout(to1, topo_tn_turn, -1);


    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, sone, 0, 1);
    NSLog(@"init done");
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG(
    {.to=MA3_UI_CTC, .from=MA1_CONTROL(), .cmd=CMD_TN_RESER_NOTIF, .v1=0, .v2=0},
    {.to=MA3_UI_GEN, .from=MA1_CTRL(0),   .cmd=CMD_TRSTATE_NOTIF, .v1=train_state_station, .v2=0});
}

- (void)tearDown {
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




- (void)testStartRightNormal
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 12;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    // {80, F0, C3, 2, 0},
    // {80, 90, 20, 257, 511},
    //{80, 90, 24, 90, 0},
    //{80, 00, 44, 1485, 1025},
    //{80, F0, C3, 1, 2}
    EXPMSG_ITRIG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0},
    {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C4, .subc=0x55,       .vb0=1, .vb1=0xFF, .vb2=0xFF, .vb3=0xFF},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=1485, .vcu8=tag_chkocc, .vb8=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=585, .vcu8=tag_reserve_c2, .vb8=1},
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=2});
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
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0});
}

- (void)testStartRightBrake1 {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 80;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG_ITRIG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0},
            {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C4, .subc=0x55,        .vb0=1, .vb1=0xFF, .vb2=0xFF, .vb3=0xFF},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
            {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_BRAKE, .v1=315, .subc=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=855, .vcu8=tag_chkocc, .vb8=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=315, .vcu8=tag_stop_eot, .vb8=1},
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=2});
}

- (void)testStartRightBrake2 {
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 78;
    tvars._curposmm = 30;
    [self startRight];
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG_ITRIG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C4, .subc=0x55,        .vb0=1, .vb1=0xFF, .vb2=0xFF, .vb3=0xFF},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
                 {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_BRAKE, .v1=405, .subc=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=945, .vcu8=tag_chkocc, .vb8=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=405, .vcu8=tag_stop_eot, .vb8=1},
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=2});
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
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s); //.{80, F0, C3, 3, 2}
    EXPMSG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0},
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_blkwait, .v2=train_state_station});
}


- (void) testChangeDir
{
    [self testStartRightNormal];
    ctrl3_upcmd_set_desired_speed(0, &tvars, -90);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s); // {80, 90, 24, 0, 0}
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});

    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);
    XCTAssert(tvars._sdir == 0);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
    s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    // {80, F0, C3, 2, 1}
    EXPMSG({.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_station, .v2=train_state_running},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C4, .subc=0,        .vb0=1, .vb1=0xFF, .vb2=0xFF, .vb3=0xFF});

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
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s); // {80, F0, C3, 3, 2}
    EXPMSG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0},
    {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_blkwait, .v2=train_state_station});

    // train is blk wait on turn out 1, change turnout 1
    // and it should start
    topology_set_turnout(to1, topo_tn_turn, -1);
    ctrl3_occupency_updated(0, &tvars);
    
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._target_unisgned_speed == 90);
    s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    // {80, 90, 20, 257, 511},{80, 90, 24, 90, 0},{80, 00, 44, 225, 1025},{80, 00, 44, 1395, 1281},{80, F0, C3, 1, 3}
    EXPMSG_ITRIG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C4, .subc=0x55,        .vb0=1, .vb1=0xFF, .vb2=0xFF, .vb3=0xFF},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=225, .vcu8=tag_chkocc, .vb8=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=1395, .vcu8=tag_brake, .vb8=1},
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_running, .v2=train_state_blkwait});
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
    
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    
    //EXPMSG({.from=MA1_CONTROL(), .to=MA3_UI_CTC, .cmd=CMD_TN_RESER_NOTIF, .v1=1, .v2=0});
    EXPMSG_NONE();
}







- (void) testStop
{
    [self testStartRightNormal];
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    ctrl3_upcmd_set_desired_speed_zero(0, &tvars);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == 1);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});

    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);
    XCTAssert(tvars._sdir == 0);
    XCTAssert(tvars._target_unisgned_speed == 0);
    XCTAssert(tvars._desired_signed_speed == 0);
    s = dump_msgbuf(0);
    NSLog(@"...%@", s); // {80, 90, 24, 0, 0},{80, F0, C3, 2, 1}
    EXPMSG({.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_station, .v2=train_state_running},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .subc=0, .cmd=CMD_SET_C4, .vb0=1, .vb1=0xFF, .vb2=0xFF, .vb3=0xFF});

}

// ---------------------------------------------------------


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

- (void)testStartLeft1 {
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


- (void)testStartLeft {
    tconf->trainlen_left_cm = 5;
    tconf->trainlen_right_cm = 19;
    tvars._curposmm = 30;
    
    XCTAssert(tvars._state == train_state_station);
    
    ctrl3_stop_detected(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    ctrl3_upcmd_set_desired_speed_zero(0, &tvars);
    XCTAssert(tvars._state == train_state_station);

    // start left
    ctrl3_upcmd_set_desired_speed(0, &tvars, -90);
    XCTAssert(tvars._state == train_state_running);
    XCTAssert(tvars._sdir == -1);
    XCTAssert(tvars._target_unisgned_speed == 90);
    XCTAssert(tvars._desired_signed_speed == -90);
    XCTAssert(tvars._spd_limit == 99);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    // {80, 90, 20, -255, -256},{80, 90, 24, 90, 0},{80, F0, C3, 1, 2}
    // {80, 90, 20, 1, 0},{80, 90, 20, -255, -256},{80, 90, 24, 90, 0},{80, F0, C3, 1, 2}
    EXPMSG_ITRIG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .subc=0, .cmd=CMD_SET_C4, .vb0=1, .vb1=0, .vb2=0xFF, .vb3=0xFF},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .subc=0, .cmd=CMD_SET_C4,    .subc=0xAA,     .vb0=1, .vb1=0, .vb2=0xFF, .vb3=0xFF},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
           /*{.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=945, .vcu8=tag_chkocc, .vb8=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=405, .vcu8=tag_stop_eot, .vb8=1},*/
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=2});
}


@end

// ---------------------------------------------------------


