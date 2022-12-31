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



static NSString *dump_msgbuf(int clear);
static int compareMsg64(const msg_64_t *exp, int n, int clear);

#define EXPMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    int rcc = compareMsg64(exp, n, 1);                        \
    XCTAssert(!rcc);                                          \
} while (0)

#define EXPMSG_NONE() do {                                     \
    int rcc = compareMsg64(NULL, 0, 1);                        \
    XCTAssert(!rcc);                                          \
} while (0)

static msg_64_t qbuf[16];

mqf_t from_ctrl =  {
    .head=0,
    .tail=0,
    .msgsiz=sizeof(msg_64_t),
    .num=16,
    .maxuse=0,
    .msgbuf=(uint8_t *) qbuf,
    .silentdrop=0
    
};

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
extern int errorhandler;

- (void)setUp
{
    mqf_clear(&from_ctrl);
    errorhandler = 0;
    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topology_set_turnout(to0, topo_tn_straight, -1);
    topology_set_turnout(to1, topo_tn_turn, -1);


    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, sone, 1);
    NSLog(@"init done");
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
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=1485, .vcu8=4, .vb8=1},
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
    EXPMSG_NONE();
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
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=40, .v2=0},
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
    XCTAssert(tvars._target_unisgned_speed == 60);
    XCTAssert(tvars._desired_signed_speed == 90);
    XCTAssert(tvars._spd_limit == 99);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=60, .v2=0},
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
    EXPMSG({.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_blkwait, .v2=train_state_station});
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
    EXPMSG({.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_station, .v2=train_state_running});

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
    EXPMSG({.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_blkwait, .v2=train_state_station});

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
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1},
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
    EXPMSG({.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=train_state_station, .v2=train_state_running});

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
    EXPMSG({.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=-1, .vb2=0xFF, .vb3=-1},
           {.to=MA1_SPDCTL(0),   .from=MA1_CTRL(0), .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0},
           /*{.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=945, .vcu8=tag_chkocc, .vb8=1},
           {.to=MA0_CANTON(0), .subc=1, .from=MA1_CTRL(0), .cmd=CMD_POSE_SET_TRIG, .va16=405, .vcu8=tag_stop_eot, .vb8=1},*/
           {.to=MA3_UI_GEN,      .from=MA1_CTRL(0), .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=2});
}


@end

// ---------------------------------------------------------

static NSString *dump_msgbuf(int clear)
{
    NSString *r = @"";
    int first = 1;
    for (int i=0; i<from_ctrl.head; i++) {
        r = [r stringByAppendingFormat:@"%s{%2.2X, %2.2X, %2.2X, %d, %d}",
             first ? "" : ",",
             qbuf[i].from,
             qbuf[i].to,
             qbuf[i].cmd,
             qbuf[i].v1,
             qbuf[i].v2 ];
        first = 0;
    }
    if (clear) {
        mqf_clear(&from_ctrl);
    }
    return r;
}


static int compareMsg64(const msg_64_t *exp, int n, int clear)
{
    int rc = 0;
    if (mqf_len(&from_ctrl) != n) {
        NSLog(@"expect %d msg, got %d", n, mqf_len(&from_ctrl));
        rc = -2;
    } else {
        for (int i=0; i<n; i++) {
            // per msg compare, for easier debug
            if (memcmp(&qbuf[i], &exp[i], sizeof(msg_64_t))) {
                NSLog(@"%d exp: %2.2x %2.2x cmd=%2.2x subc=%d v1=%d v2=%d", i,
                      exp[i].from, exp[i].to, exp[i].cmd, exp[i].subc, exp[i].v1, exp[i].v2);
                NSLog(@"%d got: %2.2x %2.2x cmd=%2.2x subc=%d v1=%d v2=%d", i,
                      qbuf[i].from, qbuf[i].to, qbuf[i].cmd, qbuf[i].subc, qbuf[i].v1, qbuf[i].v2);
                rc = i+1;
                break;
            }
        }
    }
    if (clear) {
        mqf_clear(&from_ctrl);
    }
    return rc;
}
