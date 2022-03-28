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
    tconf->trainlen_right_cm = 19;
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



- (void) test_chk_front_right
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
    rettrigs_t rettrigs;
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { {44, tag_chkocc}, {0, 0}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
    
    // or..
    // train cannot goes right (or only 4cm before margin) because turnout 0 is in bad position
    topology_set_turnout(0, 0, -1);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    const rettrigs_t expt2 = { {44, tag_chkocc}, {0, 0}, {32,tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt2, sizeof(rettrigs_t)));
    
    tvars._curposmm = 310;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_cm == 13);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    const rettrigs_t expt3 = { {44, tag_chkocc}, {0, 0}, {31+1,tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt3, sizeof(rettrigs_t)));
    
    tvars._curposmm = 290;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_cm == 15);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    const rettrigs_t expt4 = { {44, tag_chkocc}, {0, 0}, {29+3,tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt4, sizeof(rettrigs_t)));
}

- (void) testProgressRight
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
    topology_set_turnout(0, 1, -1);
    
    rettrigs_t rettrigs;
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { {44, tag_chkocc}, {0, 0}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
    
    // ------------------------
    // trains progress until triggering of tag_chkocc
    tvars._curposmm = rettrigs[0].poscm*10; // 441
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt2 = { {67, tag_chkocc}, {0, 0}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt2, sizeof(rettrigs_t)));

    // if s1 is locked
    topology_set_turnout(1, 0, -1);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt3 = { {67, tag_chkocc}, {44+18, tag_brake}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt3, sizeof(rettrigs_t)));
   
    // ------------
    // c1sblk = b2 = 70cm, so next trig is tag_chkocc again (we ignore brake)
    
    tvars._curposmm = rettrigs[0].poscm*10; // 670
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
       
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==10);
    const rettrigs_t expt4 = { {0, 0}, {0, 0}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt4, sizeof(rettrigs_t)));
    // not trig because brake already reached and next is c1 change
    
    
    tvars.c1_sblk.n = 3;
    tvars._curposmm = 0;
    tvars.beginposmm = 0;
    rc = ctrl2_update_front_sblks_c1changed(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==7);
    const rettrigs_t expt5 = { {19, tag_chkocc}, {0, 0}, {7,tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt5, sizeof(rettrigs_t)));
    

}
@end
