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
static lsblk_num_t szero = {0};
static lsblk_num_t sone = {1};
static lsblk_num_t sthree = {3};
static lsblk_num_t sfoor = {4};
static lsblk_num_t sfive = {5};

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


static int check_occupency3(int b1, int b2, int b3)
{
    for (int i=0; i<15; i++) {
        int s = get_block_addr_occupency(i);
        int expocc = ((i==b1)||(i==b2)||(i==b3)) ? 1 : 0;
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
static int check_occupency(int b1, int b2)
{
    return check_occupency3(b1, b2, -1);
}
static int check_occupency3d(int b1, int b2, int b3)
{
    for (int i=0; i<15; i++) {
        int s = get_block_addr_occupency(i);
        int expocc = ((i==b1)||(i==b2)||(i==b3)) ? 1 : 0;
        if (!expocc) {
            if (BLK_OCC_FREE != s) return -1;
        } else {
            if (BLK_OCC_FREE == s) return -1;
        }
    }
    return 0;
}

static void purge_block_delayed(void)
{
    static int dc = 0;
    for (int i=0; i<20; i++) {
        check_block_delayed(dc);
        dc += 100;
    }
}


void ctrl2_set_turnout(int tn, int v)
{
}
void ctrl2_send_led(uint8_t led_num, uint8_t prog_num)
{
}



@implementation TestCtrlP {
    train_ctrl_t tvars;
    const train_config_t *tconf;
}

- (void)setUp
{
    tconf = get_train_cnf(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topolgy_set_turnout(0, 0);
    topolgy_set_turnout(1, 0);


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
    ctrl_set_timer(0, &tvars, TAUTO, 58);
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

    NSString *s = dump_msgbuf(0);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG_U1, .v32=142},
           {.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0, .v32=-40});


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
    //XCTAssert(rc==2);
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
    //  c0 <-- c1
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, -82);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    s = dump_msgbuf(0);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=82, .v2=0});
    XCTAssert(tvars._target_speed == 82);
    XCTAssert(tvars._dir == -1);
    XCTAssert(tvars.can2_addr == 0x00);
    XCTAssert(0==check_occupency(1, 0));
    //  c0 <-- c1
    
    XCTAssert(tvars.c1c2 == 0);
    ctrl2_evt_entered_c2(0, &tvars, 1);
    XCTAssert(tvars.c1c2 == 1);
    ctrl2_evt_leaved_c1(0, &tvars);
    //  <- c0 -
    int l0 = get_lsblk_len(szero, NULL);
    
    XCTAssert(tvars.c1c2 == 0);
    XCTAssert(tvars.can2_addr == 0xFF);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    //{D0, C8, 51, -840, -1},{D0, C8, 11, -256, -1},{D0, C8, 10, 70, 0}
    // CMD_POSE_SET_TRIG2
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=-1, .vb2=0xFF, .vb3=-1}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-1720}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=70, .v2=0});
    XCTAssert(tvars.pose2_set);
    XCTAssert(tvars.trig_eoseg==0);
    XCTAssert(tvars._dir==-1);
    XCTAssert(tvars._target_speed == 70);
    XCTAssert(0==check_occupency(0, -1));

    XCTAssert(tvars.trig_eoseg==0);
    ctrl2_evt_pose_triggered(0, &tvars, 0x00, (1<<0), 30);
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
    //NSString *s1 = dump_msgbuf(0);
    // {D0, C8, 11, 0, 1}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=0, .vb2=0xFF, .vb3=0});
    
    // delayed free for block1
    purge_block_delayed();

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

- (void) testTrainLeft2BlkNotFreed
{
    [self testTrainStartLeft];
    
    // do stop
    ctrl2_evt_stop_detected(0, &tvars, 23);
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    XCTAssert(tvars._dir == 0);
    XCTAssert(0==check_occupency(0, -1));
    NSString *s1 = dump_msgbuf(0);
    // {D0, C8, 11, 0, 1}
    // {D0, C8, 11, 0, 255},{D0, 81, 26, 2, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=0, .vb2=0xFF, .vb3=0});
    
    // do NOT delayed free for block1
    // thus going right will be forbidden due to block occupied
    //    purge_block_delayed();


    // now go right
    ctrl2_upcmd_set_desired_speed(0, &tvars, 90);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    //NSString *s = dump_msgbuf(0);
    // {D0, C8, 11, 256, 257},{D0, 81, 26, 1, 0},{D0, C8, 10, 70, 0}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=3, .v2=0});
    XCTAssert(tvars._dir==1);
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(tvars.can2_addr = 0xFF);
    XCTAssert(tvars._state == train_blk_wait);
    XCTAssert(0==check_occupency(0, -1));
    
    // free the block, the train should start
    purge_block_delayed();

    XCTAssert(0==check_occupency3d(0, -1, -1));
    
    rc = ctrl2_tick_process(0, &tvars, tconf, 1);
    XCTAssert(rc==3);
    //NSString *s2 = dump_msgbuf(0);
    // {D0, C8, 11, 256, 257},{D0, 81, 26, 1, 0},{D0, C8, 10, 90, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=1, .vb2=1, .vb3=1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=90, .v2=0});
    XCTAssert(tvars._dir==1);
    XCTAssert(tvars._target_speed == 90);
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
    int l1 = get_lsblk_len(sone, NULL);
    NSString *s = dump_msgbuf(0);
    //{D0, C8, 11, 257, 511},{D0, 81, 26, 1, 0},{D0, C8, 51, 420, 0},{D0, C8, 10, 70, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=1, .vb1=1, .vb2=0xFF, .vb3=1}
           ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
           ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=660}
           ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=70, .v2=0});
    XCTAssert(tvars.trig_eoseg==0);
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
    
    XCTAssert(tvars.trig_eoseg==0);
    ctrl2_evt_pose_triggered(0, &tvars, 0x01, (1<<0), 30);
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


- (void) testRightBlockWaitAndRestartWithStopDetected
{
    [self testTrainStartRight];
    
    XCTAssert(tvars.trig_eoseg==0);
    ctrl2_evt_pose_triggered(0, &tvars, 0x01, (1<<0), 30);
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
    
    ctrl2_evt_stop_detected(0, &tvars, 333);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    NSString *st = dump_msgbuf(1);
    XCTAssert(tvars._state == train_blk_wait);
    XCTAssert(tvars._dir == 0); // because stopped
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(0==check_occupency(1, -1));

    
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
    //NSString *s = dump_msgbuf(0);
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
    XCTAssert(tvars.trig_eoseg==0);
    ctrl2_evt_pose_triggered(0, &tvars, 0x01, (1<<0), 30);
    rc = ctrl2_tick_process(0, &tvars, tconf, 1);
    //XCTAssert(rc==0);
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 92);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.can2_addr == 0x03);
    XCTAssert(tvars.spd_limit == 100);
}

- (void) testTrainStartC2RightBlk
{
    topolgy_set_turnout(0, 0);
    topolgy_set_turnout(1, 1); // route from c0-c1-c3
    tvars.c1_sblk = szero;
    tvars.can1_addr = 0x00;
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    dump_msgbuf(1);
    check_occupency(0, -1);
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, 92);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    //XCTAssert(rc==3);
    NSString *s = dump_msgbuf(0);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=0, .vb1=1, .vb2=1, .vb3=1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=92, .v2=0});
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(!tvars.pose2_set);
    XCTAssert(tvars._dir == 1);
    XCTAssert(tvars._target_speed == 92);
    XCTAssert(tvars.can1_addr == 0x00);
    XCTAssert(tvars.can2_addr == 0x01);
    XCTAssert(0==check_occupency(0,1));

    // stop
    ctrl2_upcmd_set_desired_speed(0, &tvars, 0);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    s = dump_msgbuf(0);
    XCTAssert(tvars.can1_addr == 0x00);
    XCTAssert(tvars.can2_addr == 0x01);
    XCTAssert(0==check_occupency(0,1));
    
    ctrl2_evt_stop_detected(0, &tvars, 333);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    s = dump_msgbuf(0);
    XCTAssert(tvars.can1_addr == 0x00);
    XCTAssert(tvars.can2_addr == 0xFF);
    XCTAssert(0==check_occupency(0, -1));

    

    // sudently set blk 1 occupied
    topology_or_occupency_changed = 0;
    set_block_addr_occupency(1, BLK_OCC_STOP, 1, snone);
    XCTAssert(topology_or_occupency_changed);
    
    // try to restart
    ctrl2_upcmd_set_desired_speed(0, &tvars, 90);
    rc = ctrl2_tick_process(0, &tvars, tconf, 1);
    XCTAssert(rc==2);
    s = dump_msgbuf(0);
    // {D0, C8, 10, 0, 0},{D0, C8, 11, 0, 255},{D0, 81, 26, 2, 0},{D0, C8, 11, 256, 257},{D0, 81, 26, 1, 0},{D0, C8, 10, 90, 0}
    XCTAssert(tvars.can2_addr == 0xFF);
    
    
}

- (void) testSubPose1
{
    train_config_t myconf;
    memcpy(&myconf, tconf, sizeof(myconf));
    myconf.pose_per_cm = 20;
    
    topolgy_set_turnout(1, 0);
    topolgy_set_turnout(2, 1);
    ctrl2_init_train(0, &tvars, sthree);
    int rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    XCTAssert(rc==1);
    NSString *s = dump_msgbuf(1);
    
    train_ctrl_t savtvar = tvars;
    int l3 = 10*get_lsblk_len(sthree, NULL);
    int l4 = 10*get_lsblk_len(sfoor, NULL);
    int l5 = 10*get_lsblk_len(sfive, NULL);
    XCTAssert(l3==540);
    XCTAssert(l4==800);
    
    //tvars.curposmm = 160;
    tvars.beginposmm = 0;
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, -30);
    rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=3, .vb1=-1, .vb2=0xFF, .vb3=-1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=1}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=30, .v2=0});
    XCTAssert(tvars.trig_eoseg==1);

    
    tvars = savtvar;
    tvars.beginposmm = -1000;
    
    ctrl2_upcmd_set_desired_speed(0, &tvars, -30);
    rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=3, .vb1=-1, .vb2=0xFF, .vb3=-1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-2000}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=30, .v2=0});
    XCTAssert(tvars.trig_eoseg==1);

    XCTAssert(tvars.trig_eoseg==1);
    rc  = ctrl2_evt_pose_triggered(0, &tvars, 0x03, (1<<0), -204);
    XCTAssert(!rc);
    XCTAssert(tvars.curposmm==-1020);
    XCTAssert(tvars.beginposmm==-1000-800);
    
    rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    XCTAssert(rc==2);
    s = dump_msgbuf(0);
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-3600});
    XCTAssert(tvars.trig_eoseg==1);
    XCTAssert(tvars.curposmm==-1020);
    XCTAssert(tvars.beginposmm==-1780);
    
    ctrl2_evt_stop_detected(0, &tvars, -3000);
    XCTAssert(tvars.curposmm==-1500);
    XCTAssert(tvars.beginposmm==-1780);

    rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    //XCTAssert(rc==2);
    s = dump_msgbuf(1);
    // {D0, C8, 11, 3, 255},{D0, 81, 26, 2, 0},{D0, C8, 10, 0, 0}

    // now right
    ctrl2_upcmd_set_desired_speed(0, &tvars, 42);
    rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
     //{D0, C8, 11, 259, 511},{D0, 81, 26, 1, 0},{D0, C8, 50, -2000, -1},{D0, C8, 10, 42, 0}


    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=3, .vb1=1, .vb2=0xFF, .vb3=1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-2000}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=42, .v2=0});
    XCTAssert(tvars.trig_eoseg==1);

    XCTAssert(tvars.curposmm==-1500);
    XCTAssert(tvars.beginposmm==-1780);
    
    XCTAssert(tvars.trig_eoseg==1);
    rc  = ctrl2_evt_pose_triggered(0, &tvars, 0x03, (1<<0), -199);
    XCTAssert(!rc);
    XCTAssert(tvars.curposmm==-995);
    XCTAssert(tvars.beginposmm==-1000);
    
    rc = ctrl2_tick_process(0, &tvars, &myconf, 0);
    XCTAssert(rc==2);
    s = dump_msgbuf(0);
      // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
      EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-1160});
    XCTAssert(tvars.trig_eoseg==0);

      XCTAssert(tvars.curposmm==-995);
      XCTAssert(tvars.beginposmm==-1000);

}
    

- (void) testSub1
{
    topolgy_set_turnout(1, 0);
    topolgy_set_turnout(2, 1);
    ctrl2_init_train(0, &tvars, sthree);
    int rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==1);
    NSString *s = dump_msgbuf(1);
    //{D0, C8, 11, 3, 255},{D0, 81, 26, 2, 0},{D0, C8, 10, 0, 0}

    // go left
    ctrl2_upcmd_set_desired_speed(0, &tvars, -82);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    // {D0, C8, 11, -253, -1},{D0, 81, 26, 1, 0},{D0, C8, 50, -640, -1},{D0, C8, 10, 82, 0}
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2, .vb0=3, .vb1=-1, .vb2=0xFF, .vb3=-1}
          ,{.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=1, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=1}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=82, .v2=0});
    
    XCTAssert(tvars.trig_eoseg==1);
    XCTAssert(tvars._target_speed == 82);
    XCTAssert(tvars._dir == -1);
    XCTAssert(tvars.can2_addr == 0xFF);
    XCTAssert(tvars.pose2_set==0);
    XCTAssert(0==check_occupency(3, -1));
    uint8_t rsblk, rtrn;
    occupency_block_addr_info(3, &rtrn, &rsblk);
    XCTAssert(rtrn==0);
    XCTAssert(rsblk==3);
    
    XCTAssert(tvars.trig_eoseg==1);
    ctrl2_evt_pose_triggered(0, &tvars, 0x03, (1<<0), -30);

    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    s = dump_msgbuf(0);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-1560});
    XCTAssert(tvars.trig_eoseg==1);
    XCTAssert(tvars._dir==-1);
    XCTAssert(tvars._target_speed == 82);
    XCTAssert(0==check_occupency(3, -1));
    occupency_block_addr_info(3, NULL, &rsblk);
    XCTAssert(rsblk==4);
    
    XCTAssert(tvars.trig_eoseg==1);
    ctrl2_evt_pose_triggered(0, &tvars, 0x03, (1<<0), -70);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    occupency_block_addr_info(3, NULL, &rsblk);
    XCTAssert(rsblk==5);
    EXPMSG({.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_POSE_SET_TRIG0,   .v32=-2140}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=70, .v2=0});
    XCTAssert(tvars.trig_eoseg==0);
    XCTAssert(tvars._dir==-1);
    XCTAssert(tvars._target_speed == 70);
    XCTAssert(0==check_occupency(3, -1));
    XCTAssert(tvars.pose2_set==1);
    
    XCTAssert(tvars.trig_eoseg==0);
    ctrl2_evt_pose_triggered(0, &tvars, 0x03, (1<<0), 30);
    XCTAssert(tvars.pose2_set == 0);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==3);
    s = dump_msgbuf(0);
    // {D0, 81, 26, 4, 0},{D0, C8, 10, 0, 0}
    EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=4, .v2=0}
          ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_TARGET_SPEED, .v1=0, .v2=0});
    XCTAssert(!tvars.pose2_set);
    XCTAssert(tvars._dir==-1);
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(tvars._state == train_end_of_track);
    
    ctrl2_evt_stop_detected(0, &tvars, 333);
    rc = ctrl2_tick_process(0, &tvars, tconf, 0);
    XCTAssert(rc==2);
    XCTAssert(tvars._dir == 0);
    //XCTAssert(tvars._state == train_station);
    XCTAssert(0==check_occupency(3, -1));
    //NSString *s1 = dump_msgbuf(0);
    // {D0, 81, 26, 4, 0},{D0, C8, 10, 0, 0}
    //EXPMSG({.to=MA_UI(UISUB_TFT), .from=0xD0, .cmd=CMD_TRSTATE_NOTIF,    .v1=2, .v2=0}
    //      ,{.to=MA_TRAIN_SC(0),   .from=0xD0, .cmd=CMD_SET_C1_C2,        .vb0=3, .vb1=0, .vb2=0xFF, .vb3=0});
    
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

static const train_config_t testconf = {
    .pose_per_cm = 20,
    .slipping = 100,
};


const train_config_t *get_train_cnf(int idx)
{
    return &testconf;
}

@end
