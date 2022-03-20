//
//  TestLongTrain.m
//  trainctlTests
//
//  Created by danielbraun on 20/03/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlP.h"


@interface TestLongTrain : XCTestCase

@end

@implementation TestLongTrain {
    train_ctrl_t tvars;
    train_config_t *tconf;
}


static lsblk_num_t snone = {-1};
static lsblk_num_t szero = {0};
static lsblk_num_t sone = {1};


- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    
    extern uint8_t topology_num;
    topology_num = 1;

    tconf = (train_config_t *) get_train_cnf(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topology_set_turnout(0, 0, -1);
    topology_set_turnout(1, 1, -1);


    tvars._mode = train_manual;
    ctrl2_init_train(0, &tvars, sone);
    NSLog(@"init done");
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void) test1
{
    tconf->trainlen_left = 0;
    tconf->trainlen_right = 90;
    tvars.curposmm = 30;
    lsblk_num_t r[4];
    int n = ctrl2_get_next_sblk(0, &tvars, tconf, 0, r, 4, 0);
    XCTAssert(n==1);
}
@end
