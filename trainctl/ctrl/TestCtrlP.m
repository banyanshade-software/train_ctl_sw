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

@interface TestCtrlP : XCTestCase

@end

static NSString *dump_msgbuf(int clear);

@implementation TestCtrlP {
    train_ctrl_t tvars;
}

- (void)setUp
{
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    tvars._dir = 0;
    tvars._mode = train_manual;
    tvars._state = train_off;
    tvars._target_speed = 0;
    tvars.c1_sblk.n = 1;
    tvars.can1_addr = canton_for_lsblk(tvars.c1_sblk);
    tvars.can2_addr = 0xFF;
    tvars.behaviour_flags = 0;
    tvars.desired_speed = 0;
}

- (void)tearDown {
}

- (void)testState {
    XCTAssert(sizeof(lsblk_num_t)==1);
    
    ctrl_set_state(0, &tvars, train_station);
    
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==1); // notif ui
    //NSString *s = dump_msgbuf(0);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D0, 81, 26, 3, 0}"]);

    // same state : no notif ui
    ctrl_set_state(0, &tvars, train_station);
    XCTAssert(train_station == tvars._state);
    XCTAssert(mqf_len(&from_ctrl)==0);
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

- (void) test_dir_tspeed
{
    tvars._dir = 0;
    ctrl_set_dir(0, &tvars, 0, 0);
    XCTAssert(mqf_len(&from_ctrl)==0);
    ctrl_set_dir(0, &tvars, 0, 1);
    XCTAssert(mqf_len(&from_ctrl)==1);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D0, 81, 23, 0, 0}"]);
    ctrl_set_tspeed(0, &tvars, 23);
    XCTAssert(mqf_len(&from_ctrl)==2);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D0, 81, 24, 23, 0},{D0, C8, 10, 23, 0}"]);
    ctrl_set_tspeed(0, &tvars, 23);
    XCTAssert(mqf_len(&from_ctrl)==0);
}

- (void) test_set_trig
{
    ctrl_set_pose_trig(0, 142, 1);
    XCTAssert(mqf_len(&from_ctrl)==1);
    ctrl_set_pose_trig(0, -40, 1);
    XCTAssert(mqf_len(&from_ctrl)==2);

    void ctrl_set_pose_trig(int numtrain, int32_t pose, int trignum);
    //NSString *s = dump_msgbuf(0);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D0, C8, 51, 142, 0},{D0, C8, 51, -40, -1}"]);

}


- (void) test_update_c2
{
    tvars._dir = 1;
    tvars._target_speed = 90;
    tvars._state = train_station;
    ctrl_update_c2_state_limits(0, &tvars, get_train_cnf(0), upd_init);
    // _dir and _target_speed should have been reseted becaunse train_station state
    XCTAssert(tvars._target_speed == 0);
    XCTAssert(tvars._dir == 0);
    //NSString *s = dump_msgbuf(0);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D0, C8, 11, 1, 255}"]);

    tvars._state = train_running_c1;
    tvars._dir = 1;
    tvars._target_speed = 90;
    XCTAssert(tvars.can2_addr == 0xFF);
    
    ctrl_update_c2_state_limits(0, &tvars, get_train_cnf(0), upd_change_dir);
    XCTAssert(tvars.can2_addr == 0x00);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(tvars._state == train_running_c1);
    XCTAssert(get_block_addr_occupency(0)==BLK_OCC_C2);
    NSString *s = dump_msgbuf(0);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D0, C8, 11, 1, 255}"]);

    
    set_block_addr_occupency(tvars.can2_addr, occupied(tvars._dir), 0  ,
                             first_lsblk_with_canton(tvars.can2_addr, tvars.c1_sblk));
    tvars._state = train_running_c1c2;
        

    tvars.c1_sblk = first_lsblk_with_canton(tvars.can2_addr, tvars.c1_sblk);
    XCTAssert(tvars.c1_sblk.n == 0);
    set_block_addr_occupency(tvars.can1_addr, BLK_OCC_FREE, 0xFF, snone);
    tvars.can1_addr = tvars.can2_addr;
    
    tvars._state = train_running_c1;
    ctrl_update_c2_state_limits(0, &tvars, get_train_cnf(0), upd_c1c2);
    XCTAssert(tvars.spd_limit == 100);
    XCTAssert(tvars.can2_addr == 0x01);
    XCTAssert(get_block_addr_occupency(1)==BLK_OCC_C2);
    NSString *s2 = dump_msgbuf(0);
    XCTAssert([dump_msgbuf(1) isEqualToString:@"{D7, 82, A2, 768, 0},{D7, 82, A2, 1, 255},{D7, 82, A2, 1025, 0},{D0, C8, 11, 256, 257},{D0, 81, 24, 0, 1},{D0, C8, 10, 0, 1}"]);

}
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
