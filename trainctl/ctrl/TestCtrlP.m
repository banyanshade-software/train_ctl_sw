//
//  TestCtrlP.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 28/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlP.h"


static lsblk_num_t snone = {-1};
static lsblk_num_t sone = {1};

@interface TestCtrlP : XCTestCase

@end

static NSString *dump_msgbuf(int clear);
static int compareMsg64(const msg_64_t *exp, int n, int clear);

#define EXPMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    printf("n=%d",n);                                        \
    int rc = compareMsg64(exp, n, 1);                        \
    XCTAssert(!rc);                                          \
} while (0)



@implementation TestCtrlP {
    train_ctrl_t tvars;
    const train_config_t *tconf;
}

- (void)setUp
{
    tconf = get_train_cnf(0);
    notify_occupency_change = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topolgy_set_turnout(0, 0);
    topolgy_set_turnout(1, 0);

#ifdef OLD_CTRL

    tvars._dir = 0;
    tvars._mode = train_manual;
    tvars._state = train_off;
    tvars._target_speed = 0;
    tvars.c1_sblk.n = 1;
    tvars.can1_addr = canton_for_lsblk(tvars.c1_sblk);
    tvars.can2_addr = 0xFF;
    tvars.behaviour_flags = 0;
    tvars.desired_speed = 0;
#endif
    tvars._mode = train_manual;
    ctrl2_init_train(0, &tvars, sone);
    NSLog(@"init done");
}

- (void)tearDown {
}


- (void) testTimer
{
    SimuTick = 1234;
    
    ctrl_set_timer(0, &tvars, TLEAVE_C1, 42);
    ctrl_set_timer(0, &tvars, TBEHAVE, 58);
    XCTAssert(tvars.timertick[0]==42+1234);
    XCTAssert(tvars.timertick[1]==58+1234);
    XCTAssert(mqf_len(&from_ctrl)==0);
}




- (void) test_set_trig
{
    ctrl_set_pose_trig(0, 142, 1);
    XCTAssert(mqf_len(&from_ctrl)==1);
    ctrl_set_pose_trig(0, -40, 0);
    XCTAssert(mqf_len(&from_ctrl)==2);

    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG2, .v32=142},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG1, .v32=-40});


}


- (void)testState2 {
    XCTAssert(sizeof(lsblk_num_t)==1);
    XCTAssert(sizeof(msg_64_t)==8);
    tvars.tick_flags = 0;
    tvars._state = train_off;
    
    ctrl2_set_state(0, &tvars, train_station);
    
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==0); // notif ui done after
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    XCTAssert(mqf_len(&from_ctrl)==1); // notif ui
    //NSString *s = dump_msgbuf(0);
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF, .v1=2, .v2=0});

    // same state : no notif ui
    ctrl2_set_state(0, &tvars, train_station);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==0);
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==0);
    
    ctrl2_set_state(0, &tvars, train_off);
    ctrl2_set_state(0, &tvars, train_station);
    ctrl2_set_state(0, &tvars, train_running_c1);
    ctrl2_set_state(0, &tvars, train_station);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==1);
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF, .v1=2, .v2=0});
}

/*

 #define BLK_OCC_FREE    0x00
 #define BLK_OCC_STOP    0x01
 #define BLK_OCC_LEFT    0x02
 #define BLK_OCC_RIGHT    0x03
 #define BLK_OCC_C2        0x04

 #define BLK_OCC_DELAY1    0x10
 #define BLK_OCC_DELAYM    0x16
i
 */
static int check_occupency(int b1, int b2)
{
    for (int i=0; i<15; i++) {
        int s = get_block_addr_occupency(i);
        int expocc = ((i==b1)||(i==b2)) ? 1 : 0;
        switch (s) {
            case BLK_OCC_STOP:
            case BLK_OCC_LEFT:
            case BLK_OCC_RIGHT:
            case BLK_OCC_C2:
                if (expocc) break;
                return -1;
                break;
            default:
                if (!expocc) break;
                return -1;
                break;
        }
    }
    return 0;
}

- (void) testTrainInit
{
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    //NSString *s = dump_msgbuf(0);
    // {D0, C8, 11, 1, 255},{D0, 81, 26, 2, 0},{D0, C8, 10, 0, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=0, .vb2=0xFF, .vb3=0},
           {.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=2, .v2=0},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});
    XCTAssert(0==check_occupency(1, -1));
}


- (void) testTrainStartLeft
{
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    dump_msgbuf(1);
    // {D0, C8, 11, 1, 255},{D0, 81, 26, 2, 0},{D0, C8, 10, 0, 0}
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, -92);
    XCTAssert(tvars.desired_speed==-92);
    XCTAssert(tvars.tick_flags == _TFLAG_DSPD_CHANGED);
    
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    NSString *s = dump_msgbuf(0);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=-1, .vb2=0, .vb3=-1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=92, .v2=0});
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(!tvars.pose2_set);
    XCTAssert(tvars._dir == -1);
    XCTAssert(tvars._target_speed == 92);
    XCTAssert(tvars.can2_addr == 0x00);
    XCTAssert(0==check_occupency(1, 0));
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, -82);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    s = dump_msgbuf(0);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=82, .v2=0});
    XCTAssert(tvars._target_speed == 82);
    XCTAssert(tvars._dir == -1);
    XCTAssert(tvars.can2_addr == 0x00);
    XCTAssert(0==check_occupency(1, 0));

    
    XCTAssert(tvars.c1c2 == 0);
    ctrl2_evt_entered_c2(0, &tvars, 1);
    XCTAssert(tvars.c1c2 == 1);
    ctrl2_evt_leaved_c1(0, &tvars);
    XCTAssert(tvars.c1c2 == 0);
    XCTAssert(tvars.can2_addr == 0xFF);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    //{D0, C8, 51, -840, -1},{D0, C8, 11, -256, -1},{D0, C8, 10, 70, 0}
    // CMD_POSE_SET_TRIG2
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=-1, .vb2=0xFF, .vb3=-1}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG2,   .v32=-840}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=70, .v2=0});
    XCTAssert(tvars.pose2_set);
    XCTAssert(tvars._dir==-1);
    XCTAssert(tvars._target_speed == 70);
    XCTAssert(0==check_occupency(0, -1));

    ctrl2_evt_pose_triggered(0, &tvars, 0x00, 1);
    XCTAssert(!tvars.pose2_set);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    //s = dump_msgbuf(0);
    // {D0, 81, 26, 4, 0},{D0, C8, 10, 0, 0}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=4, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});
    XCTAssert(!tvars.pose2_set);
    XCTAssert(tvars._dir==-1);
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(0==check_occupency(0, -1));
}

- (void) testTrainLeft2
{
    [self testTrainStartLeft];
    
    // do stop
    ctrl2_evt_stop_detected(0, &tvars, 23);
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    XCTAssert(tvars._dir == 0);
    XCTAssert(0==check_occupency(0, -1));
    //s = dump_msgbuf(0);
    // {D0, C8, 11, 0, 1}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=0, .vb2=0xFF, .vb3=0});
        
    // now go right
    ctrl2_upcmd_set_desired_speed(0, &tvars, 90);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==4);
    //NSString *s = dump_msgbuf(0);
    // {D0, C8, 11, 256, 257},{D0, 81, 26, 1, 0},{D0, C8, 10, 70, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=1, .vb2=1, .vb3=1},
           {.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0});
    XCTAssert(tvars._dir==1);
    XCTAssert(tvars._target_speed == 90);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(tvars.can2_addr = 0x01);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(0==check_occupency(0, 1));
}


- (void) testTrainStartRight
{
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    dump_msgbuf(1);
    // {D0, C8, 11, 1, 255},{D0, 81, 26, 2, 0},{D0, C8, 10, 0, 0}
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, 92);
    XCTAssert(tvars.desired_speed==92);
    XCTAssert(tvars.tick_flags == _TFLAG_DSPD_CHANGED);
    
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==4);
    NSString *s = dump_msgbuf(0);
    //{D0, C8, 11, 257, 511},{D0, 81, 26, 1, 0},{D0, C8, 51, 420, 0},{D0, C8, 10, 70, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1}
           ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
           ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG2,   .v32=420}
           ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=70, .v2=0});
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.can2_addr == 0xFF);
    XCTAssert(tvars.spd_limit == 70);
    XCTAssert(tvars.pose2_set);
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 70);
    XCTAssert(0==check_occupency(1, -1));
}

- (void) testRightBlockWaitAndRestart
{
    [self testTrainStartRight];
    
    ctrl2_evt_pose_triggered(0, &tvars, 0x01, 1);
    XCTAssert(tvars.pose2_set == 0);
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    NSString *s = dump_msgbuf(0);
    // {D0, 81, 26, 4, 0},{D0, C8, 10, 0, 0}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=3, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});
    XCTAssert(tvars._state == train_blk_wait);
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(0==check_occupency(1, -1));
    //XCTAssert(tvars.spd_limit == 0);
    
    topolgy_set_turnout(1, 1);
    rc = ctrl2_tick_process(0, &tvars, tconf, 1);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    //{D0, C8, 11, 257, 259},{D0, 81, 26, 1, 0},{D0, C8, 10, 92, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=3, .vb3=1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=92, .v2=0});
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 92);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.can2_addr == 0x03);
    XCTAssert(0==check_occupency(1, 3));
}


- (void) testRightBlockConditionRemoved
{
    [self testTrainStartRight];
    
    topolgy_set_turnout(1, 1);
    int rc = ctrl2_tick_process(0, &tvars, tconf, 1);
    XCTAssert(rc==3);
    NSString *s = dump_msgbuf(0);
    //{D0, C8, 11, 257, 259},{D0, 81, 26, 1, 0},{D0, C8, 10, 92, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=3, .vb3=1}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=92, .v2=0});
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 92);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.can2_addr == 0x03);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(tvars.pose2_set == 0);
    XCTAssert(0==check_occupency(1, 3));
    
    // late trigger may (and will) occur and should be ignored
    ctrl2_evt_pose_triggered(0, &tvars, 0x01, 1);
    rc = ctrl2_tick_process(0, &tvars, tconf, 1);
    //XCTAssert(rc==0);
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 92);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.can2_addr == 0x03);
    XCTAssert(tvars.spd_limit == 100);
}


#ifdef OLD_CTRL


- (void)testState {
    XCTAssert(sizeof(lsblk_num_t)==1);
    XCTAssert(sizeof(msg_64_t)==8);
    
    ctrl_set_state(0, &tvars, train_station);
    
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==1); // notif ui
    NSString *s = dump_msgbuf(0);
    
    
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF, .v1=3, .v2=0});

    // same state : no notif ui
    ctrl_set_state(0, &tvars, train_station);
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==0);
}

- (void) test_dir_tspeed
{
    tvars._dir = 0;
    ctrl_set_dir(0, &tvars, 0, 0);
    XCTAssert(mqf_len(&from_ctrl)==0);
    ctrl_set_dir(0, &tvars, 0, 1);
    XCTAssert(mqf_len(&from_ctrl)==1);
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRDIR_NOTIF, .v1=0, .v2=0});

    ctrl_set_tspeed(0, &tvars, 23);
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRTSPD_NOTIF,     .v1=23, .v2=0},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=23, .v2=0});

    ctrl_set_tspeed(0, &tvars, 23);
    XCTAssert(mqf_len(&from_ctrl)==0);
}
- (void) test_update_c2
{
    topolgy_set_turnout(0, 0);
    topolgy_set_turnout(1, 0);
    tvars._dir = 1;
    tvars._target_speed = 90;
    tvars.desired_speed = 90;
    tvars.spd_limit = 99;
    tvars._state = train_station;
    ctrl_update_c2_state_limits(0, &tvars, tconf, upd_init);
    // _dir and _target_speed should have been reseted becaunse train_station state
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(tvars._dir == 0);
    EXPMSG({.to=MA_TRAIN_SC(0), .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=1, .vb1=0, .vb2=0xFF, .vb3=0});
    
    // start on sblk 1
    XCTAssert(tvars.can1_addr == 0x01);
    XCTAssert(tvars.c1_sblk.n == 1);
    
    tvars._state = train_running_c1;
    tvars._dir = 1;
    tvars.spd_limit = 100; //TODO : REMOVE should not be needed
    XCTAssert(tvars.can2_addr == 0xFF);
    
    // train should be blk wait due to switch in bad position
    ctrl_update_c2_state_limits(0, &tvars, tconf, upd_change_dir);
    XCTAssert(tvars.can2_addr == 0xFF);
    XCTAssert(tvars.spd_limit == 0);
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(tvars._state == train_blk_wait);
    //NSString *s = dump_msgbuf(0);
    //{D0, 81, 26, 4, 0},{D0, C8, 11, 257, 511}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF, .v1=4, .v2=0},
           {.to=MA_TRAIN_SC(0), .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1});

    // train should restart after turnout change
    topolgy_set_turnout(1, 1);
    ctrl_update_c2_state_limits(0, &tvars, tconf, upd_check);
    XCTAssert(tvars.can2_addr == 0x03);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(tvars._target_speed == 90);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(get_block_addr_occupency(3)==BLK_OCC_C2);
    
    //NSString *s3 = dump_msgbuf(0);
    //{D0, 81, 26, 1, 0},{D0, C8, 11, 257, 259},{D0, 81, 24, 90, 1},{D0, C8, 10, 90, 0}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF, .v1=1, .v2=0},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=1, .vb1=1, .vb2=3, .vb3=1},
           {.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRTSPD_NOTIF, .v1=90, .v2=1},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0});

    // got to c3, should stop half way due to end of trac
    tvars.c1_sblk = first_lsblk_with_canton(tvars.can2_addr, tvars.c1_sblk);
    tvars.can1_addr = tvars.can2_addr;
    tvars.can2_addr = 0xFF;
    ctrl_update_c2_state_limits(0, &tvars, tconf, upd_c1c2);
    
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.spd_limit == 70);
    XCTAssert(tvars.can2_addr == 0xFF);
    XCTAssert(tvars._target_speed == 70);
    //NSString *s3 = dump_msgbuf(0);
    //{D0, C8, 11, 259, 511},{D0, 81, 24, 70, 1},{D0, C8, 10, 70, 0},{D0, C8, 51, 320, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=3, .vb1=1, .vb2=0xFF, .vb3=1},
           {.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRTSPD_NOTIF, .v1=70, .v2=1},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=70, .v2=0},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG2, .v32=320});

    
    ctrl_update_c2_state_limits(0, &tvars, tconf, upd_pose_trig);
    XCTAssert(tvars._state == train_end_of_track);
    XCTAssert(tvars.spd_limit == 0);
    XCTAssert(tvars.can2_addr == 0xFF);
    XCTAssert(tvars._target_speed == 0);
    NSString *s3 = dump_msgbuf(0);
    //{D0, 81, 26, 5, 0},{D0, 81, 24, 0, 1},{D0, C8, 10, 0, 0}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF, .v1=5, .v2=0},
              {.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRTSPD_NOTIF, .v1=0, .v2=1},
              {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});
}
#endif
// -------------------------------------

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

static int compareMsg64(const msg_64_t *exp, int n, int clear)
{
    int rc = 0;
    if (mqf_len(&from_ctrl) != n) {
        rc = -2;
    } else {
        for (int i=0; i<n; i++) {
            // per msg compare, for easier debug
            if (memcmp(&qbuf[i], &exp[i], sizeof(msg_64_t))) {
                NSLog(@"%d exp: %2.2x %2.2x cmd=%2.2x v1=%d v2=%d", i,
                      exp[i].from, exp[i].to, exp[i].cmd, exp[i].v1, exp[i].v2);
                NSLog(@"%d got: %2.2x %2.2x cmd=%2.2x v1=%d v2=%d", i,
                      qbuf[i].from, qbuf[i].to, qbuf[i].cmd, qbuf[i].v1, qbuf[i].v2);
                rc = i;
                break;
            }
        }
    }
    if (clear) {
        mqf_clear(&from_ctrl);
    }
    return rc;
}

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
// -------------------------------------

static const train_config_t tconf = {
    .pose_per_cm = 20,
};


const train_config_t *get_train_cnf(int idx)
{
    return &tconf;
}

@end
