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
    conf_train_t *tconf;
}


static lsblk_num_t snone = {-1};
static lsblk_num_t szero = {0};
static lsblk_num_t sone = {1};
static lsblk_num_t stwo = {2};

static const xtrnaddr_t to0 = { .v = 0};
static const xtrnaddr_t to1 = { .v = 1};


- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    
    //extern uint8_t topology_num;
    //topology_num = 0;

    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    topology_set_turnout(to0, 0, -1);
    topology_set_turnout(to1, 1, -1);


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
    lsblk_num_t r[4] = {0};
    int n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    tconf->trainlen_right_cm = 30;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    tconf->trainlen_right_cm = 44;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n=1);
    
    static const int exp1[] = { 3 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    
    tconf->trainlen_right_cm = 120;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==2);
    static const int exp2[] = { 3, -1};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
}

- (void) test2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 19;
    ctrl2_init_train(0, &tvars, szero);
    tvars._curposmm = 900;
    int16_t remain = -1;
    lsblk_num_t r[4] = {0};

    // int ctrl2_get_next_sblks_(_UNUSED_ int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlen)
    int n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==1);    // len left is 9
    static const int exp1[] = { 1 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    XCTAssert(remain==98-90+45-19);
    
    tconf->trainlen_right_cm = 72;
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==2);
    static const int exp3[] = {1, 3 };
    rc = check_lsblk_array(r, exp3, n);
    XCTAssert(!rc);
    XCTAssert(remain == 98-90+45+54-72);
}

- (void) test3
{
    tconf->trainlen_left_cm = 35;
    tconf->trainlen_right_cm = 19;
    tvars._curposmm = 400; // 40cm out of 45 for s1
    int16_t remain = -1;
    lsblk_num_t r[4] = {0};
    
    int n = ctrl2_get_next_sblks_(0, &tvars, tconf, 1, r, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain == 40-35);
    
    tvars._curposmm = 50; // 5cm out of 45 for s1
    n = ctrl2_get_next_sblks_(0, &tvars, tconf, 1, r, 4, &remain);
    XCTAssert(n==1);
    static const int exp1[] = { 0 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    XCTAssert(remain==98+5-35);
}


- (void) test_chk_front_right
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    topology_set_turnout(to1, 1, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 600;             //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 90-60+45-48); // 27
    
    // train is on sblk2, cars on 3 and 1
    // (2)----(3)- - <tn0> - - (1) - - -<tn1>- -(5)-
    //      (0)  ----/          (6)------/
    
    // train is on b2 + 6cm of b3,
    
    // train can goes right because turnout 0 is in good position
    topology_set_turnout(to0, 1, -1);
    rettrigs_t rettrigs;
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
    
    
    topology_set_turnout(to1, 0, -1);
    
    memset(&tvars.rightcars, 0xFF, sizeof(tvars.rightcars));
    memset(&tvars.leftcars, 0xFF, sizeof(tvars.leftcars));
    ctrl2_get_next_sblks(0, &tvars, tconf);

    //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    // s1: front cars 18><     27cm              >
    // s1: front cars 18><15 braking><margin 12cm> (brake distance=16)
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==15);
    const rettrigs_t expt2 = { {87, tag_chkocc}, {0, 0}, {75,tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt2, sizeof(rettrigs_t)));
    
    tvars._curposmm = 600+10;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_cm == 26);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==14);
    const rettrigs_t expt3 = { {87, tag_chkocc}, {0, 0}, {75,tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt3, sizeof(rettrigs_t)));
    
    tvars._curposmm = 600+160;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_cm == 11);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc<0);
    const rettrigs_t expt4 = { {87, tag_chkocc}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt4, sizeof(rettrigs_t)));
    
   
}

- (void) testProgressRight
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    topology_set_turnout(to1, 1, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 600;             //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 27);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {0,0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
    
    // ------------------------
    // trains progress until triggering of tag_chkocc
    tvars._curposmm = rettrigs[0].poscm*10; // 870
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 90-87+45-48+54); // 54

    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt2 = { {0, 0}, {0, 0}, {0,0}};
    // no trig until c1 changes
    XCTAssert(!memcmp(rettrigs, expt2, sizeof(rettrigs_t)));
    
    // change c1
    // ----- b1 (45cm) ------+---- b3 (54cm) ----|end
    // <-----------(48)-----------> rlen = 54-3

    tvars.c1_sblk.n = 1;
    tvars._curposmm = 0;
    tvars.beginposmm = 0;
    rc = ctrl2_update_front_sblks_c1changed(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 51); // 27
    
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt5 = { {0, 0}, {23, tag_brake}, {39, tag_stop_eot}};
    XCTAssert(!memcmp(rettrigs, expt5, sizeof(rettrigs_t)));
    
    // up to first trig
    tvars._curposmm = rettrigs[1].poscm*10; // 230
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 28); // 16+12
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==16);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt6 = { {0, 0}, {0, 0}, {39, tag_stop_eot}};
    XCTAssert(!memcmp(rettrigs, expt6, sizeof(rettrigs_t)));

    // up to last trig
    tvars._curposmm = rettrigs[2].poscm*10; // 390
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 12); // 16+12
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc<0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt7 = { {0, 0}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt7, sizeof(rettrigs_t)));
}


- (void) testStartNoOccTurnout
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 1, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 620;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 25);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
}
- (void) testStartOccTurnout
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 0, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 620;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 25);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==13);
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {75, tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
}



- (void) testStartNoOccTurnout2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 1, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 750;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 12);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
}
- (void) testStartOccTurnout2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 0, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 750;
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 12);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==-1);
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
}

- (void) testProgressRight2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 0, -1);
    ctrl2_init_train(0, &tvars, stwo); // s2 90cm
    tvars._curposmm = 600;             //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    ctrl2_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 27);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==15);  // 27 cm rlen -> 12 margin + 16 brake
    const rettrigs_t expt1 = { {87, tag_chkocc}, {0, 0}, {75, tag_stop_blk_wait}};
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));
    
    // ------------------------
    // trains progress 2cm
    tvars._curposmm = 620; // (rettrigs[2].poscm-13)*10; // 602
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==13);
    // no change in trig
    XCTAssert(!memcmp(rettrigs, expt1, sizeof(rettrigs_t)));

    // go until end of brake (+15cm)
    tvars._curposmm = 600+150; // (rettrigs[2].poscm-13)*10; // 602
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==-1);
    const rettrigs_t expt2 = { {87, tag_chkocc}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt2, sizeof(rettrigs_t)));

    // change to1 to normal
    topology_set_turnout(to1, 1, -1);
    rc = ctrl2_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl2_check_front_sblks(0, &tvars, tconf, 0, rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt3 = { {87, tag_chkocc}, {0, 0}, {0, 0}};
    XCTAssert(!memcmp(rettrigs, expt3, sizeof(rettrigs_t)));
    
}
@end
