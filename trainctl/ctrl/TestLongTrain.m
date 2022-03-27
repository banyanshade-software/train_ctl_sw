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
static lsblk_num_t stwo = {2};


- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    
    extern uint8_t topology_num;
    topology_num = 0;

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


static int check_lsblk_array(const lsblk_num_t *res, const int *exp, int n)
{
    for (int i=0; i<n ; i++) {
        if (res[i].n != exp[i]) return -1;
    }
    return 0;
}

- (void) test1
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 20;
    tvars._curposmm = 30;
    lsblk_num_t r[4];
    int n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    tconf->trainlen_right_cm = 30;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==1);
    static const int exp1[] = { 4 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    
    tconf->trainlen_right_cm = 90;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==2);
    static const int exp2[] = { 4, 5 };
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
    
    
    
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 1, r, 4, NULL);
    XCTAssert(n==0);    // len left is 9
   
    
    tconf->trainlen_left_cm = 90;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 1, r, 4, NULL);
    XCTAssert(n==1);
    static const int exp3[] = { 0 };
    rc = check_lsblk_array(r, exp3, n);
    XCTAssert(!rc);
}

static void settrig(uint32_t mm, uint8_t tag)
{
    NSLog(@"set trig %d %d", mm, tag);
}

- (void) test2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 46;
    occupency_clear();
    topology_set_turnout(0, 0, -1);
    topology_set_turnout(1, 1, -1);
    ctrl2_init_train(0, &tvars, stwo);
    tvars._curposmm = 300;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 3 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 14);
    
    // train is on sblk2, cars on 3 and 1
    // (2)----(3)- - <tn0> - - (1) - - -<tn1>- -(5)-
    //      (0)  ----/          (6)------/
    
    // train is on b2 + 6cm of b3,
    
    // train can goes right because turnout 0 is in good position
    topology_set_turnout(0, 1, -1);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, settrig);
    
    // or..
    // train cannot goes right (or only 4cm before margin) because turnout 0 is in bad position
    topology_set_turnout(0, 0, -1);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, settrig);
    
    
    /* 46
     l2/8=6
     l2/10=4
     l2/12=2
     l2/14=0
     l2/18=-1
     */
}
@end
