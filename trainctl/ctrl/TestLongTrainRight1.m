//
//  TestLongTrain1.m
//  trainctlTests
//
//  Created by danielbraun on 20/03/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlLT.h"
#include "longtrain.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"

@interface TestLongTrainRight1 : XCTestCase

@end

@implementation TestLongTrainRight1 {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}


static lsblk_num_t snine = {9};
static lsblk_num_t sten = {10};
static lsblk_num_t seleven = {11};
//static lsblk_num_t stwelve = {12};


- (void)setUp {
    
    errorhandler = 0;
    //extern uint8_t topology_num;
    //topology_num = 0;

    tconf = (conf_train_t *) conf_train_get(0);
    notify_occupency_change = 0;
    ctrl_flag_notify_speed = 0;
    occupency_clear();
    mqf_clear(&from_ctrl);
    memset(&tvars, 0, sizeof(tvars));
    

    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, snine, 1);
    NSLog(@"init done");
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
}


static int check_lsblk_array(const lsblk_num_t *res, const int *exp, int n)
{
    for (int i=0; i<n ; i++) {
        if (res[i].n != exp[i]) {
            printf("i=%d res %d exp %d\n", i, res[i].n, exp[i]);
            return -1;
        }
    }
    return 0;
}

- (void) testRight1
{
    tconf->trainlen_left_cm = 0;
    
    // (A)
    tconf->trainlen_right_cm = 20;
    tvars._curposmm = 300;
    lsblk_num_t r[4] = {0};
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    // (B)
    tconf->trainlen_right_cm = 29;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    // (C)
    tconf->trainlen_right_cm = 30;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==1);
    
    static const int exp0[] = { 10 };
    int rc = check_lsblk_array(r, exp0, n);
    XCTAssert(!rc);
    
    // (D)
    tconf->trainlen_right_cm = 44;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n=1);
    
    static const int exp1[] = { 10 };
    rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    
    // (E)
    tconf->trainlen_right_cm = 100;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==3);
    static const int exp2[] = { 10, 11, 12};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
    
    // (F)
    tconf->trainlen_right_cm = 130;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==4);
    //static const int exp3[] = { 10, 11, 12, 0xFF};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(rc==-1);
    
}

- (void) testRight2
{
    tconf->trainlen_left_cm = 0;
    int16_t remain = -1;

    // (A)
    tconf->trainlen_right_cm = 20;
    tvars._curposmm = 300;
    lsblk_num_t r[4] = {0};
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain==100);
    
    // (B)
    tconf->trainlen_right_cm = 29;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain==10);

    // (C)
    tconf->trainlen_right_cm = 30;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==1);
    XCTAssert(remain==400);

    static const int exp0[] = { 10 };
    int rc = check_lsblk_array(r, exp0, n);
    XCTAssert(!rc);
    
    // (D)
    tconf->trainlen_right_cm = 44;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n=1);
    XCTAssert(remain==260);
    static const int exp1[] = { 10 };
    rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    
    // (E)
    tconf->trainlen_right_cm = 100;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==3);
    XCTAssert(remain==100);
    static const int exp2[] = { 10, 11, 12};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
    
    // (F)
    tconf->trainlen_right_cm = 130;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==4);
    //static const int exp3[] = { 10, 11, 12, 0xFF};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(rc==-1);
    XCTAssert(remain==0);
}



- (void) testRight3
{
    tconf->trainlen_left_cm = 0;
    int16_t remain = -1;

    // (A)
    tconf->trainlen_right_cm = 20;
    tvars.beginposmm = 0;
    tvars._curposmm = 300;
    lsblk_num_t r[4] = {0};
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain==100);
    
    tvars.beginposmm = 0+1112;
    tvars._curposmm = 300+1112;
    lsblk_num_t r1[4] = {0};
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r1, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain==100);

    tvars.beginposmm = 0-923;
    tvars._curposmm = 300-923;
    lsblk_num_t r2[4] = {0};
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r2, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain==100);
}

- (void) testRight4
{
    tconf->trainlen_left_cm = 0;
    int16_t remain = -1;
    int n;
    int rc;
    lsblk_num_t r[4] = {0};

    memset(r, 0, sizeof(r));
    tvars.beginposmm = 0+0;
    tvars._curposmm = 300+0;
    tconf->trainlen_right_cm = 100;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==3);
    XCTAssert(remain==100);
    static const int exp2[] = { 10, 11, 12};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
    
    
    memset(r, 0, sizeof(r));
    tvars.beginposmm = 0+342;
    tvars._curposmm = 300+342;
    tconf->trainlen_right_cm = 100;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==3);
    XCTAssert(remain==100);
    static const int exp3[] = { 10, 11, 12};
    rc = check_lsblk_array(r, exp3, n);
    XCTAssert(!rc);
    
    
    memset(r, 0, sizeof(r));
    tvars.beginposmm = 0-2123;
    tvars._curposmm = 300-2123;
    tconf->trainlen_right_cm = 100;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==3);
    XCTAssert(remain==100);
    static const int exp4[] = { 10, 11, 12};
    rc = check_lsblk_array(r, exp4, n);
    XCTAssert(!rc);
}
    
- (void) testLeft1
{
    tvars.c1_sblk = seleven;
    tconf->trainlen_left_cm = 20;
    tconf->trainlen_right_cm = 52;// not used here
    tvars._curposmm = 50; // 5cm
    int16_t remain = -1;
    lsblk_num_t r[4] = {0};
    
    // (A)
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, &remain);
    XCTAssert(n==1);
    XCTAssert(remain == 250);
    static const int exp0[] = { 10 };
    int rc = check_lsblk_array(r, exp0, n);
    XCTAssert(!rc);

    // (B)
    tconf->trainlen_left_cm = 50;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, &remain);
    XCTAssert(n==2);
    static const int exp1[] = { 10, 9 };
    rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    XCTAssert(remain==550);
}



- (void) testC2pow1
{
    [self  checkC2powerRight:0 cpos:0];
}

- (void) testC2pow2
{
    [self  checkC2powerRight:1000 cpos:0];
}

- (void) testC2pow3
{
    [self  checkC2powerRight:-1000 cpos:0];
}


- (void) checkC2powerRight:(int)bmm cpos:(int)cpos
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 18;
    occupency_clear();
    tvars.c1_sblk = sten;
    tvars.beginposmm = bmm;
    tvars._curposmm = cpos*10 + bmm;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(40-18-cpos));
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 0, 5, {{220 + bmm, tag_chkocc}, {340+bmm, tag_brake}, {120+bmm,tag_reserve_c2}, {400+bmm, tag_end_lsblk}, {300+bmm, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}


- (void) testC2pow4
{
    [self  checkC2powerRightB:0 cpos:31 ];
}
- (void) checkC2powerRightB:(int)bmm cpos:(int)cpos
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 18;
    occupency_clear();
    tvars.c1_sblk = sten;
    tvars.beginposmm = bmm;
    tvars._curposmm = (cpos*10 + bmm);
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 1);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(40+10-18-cpos));
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 1, 1, 3, {{320 + bmm, tag_chkocc}, {340+bmm, tag_brake},  {400+bmm, tag_end_lsblk}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
}

- (void) test_chk_front_right0
{
    [self checkFrontRight:0];
}

- (void) test_chk_front_right1
{
    [self checkFrontRight:100];
}
- (void) test_chk_front_right2
{
    [self checkFrontRight:-100];
}

- (void) checkFrontRight:(int)bmm
{
    int rc;
    
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 38;
    occupency_clear();
    tvars.beginposmm = bmm;
    
    // (A)
    tvars._curposmm = 200 + bmm;
    
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 0);
    XCTAssert(tvars.rightcars.rlen_mm == 20);
   
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0, 0, 1, 2, {{220+bmm, tag_chkocc}, {520+bmm, tag_reserve_c2}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));

    // (B) first trig
    tvars._curposmm = 220 + bmm;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_mm == 400);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt3 = {0, 0, 0, 1, 1, { {520+bmm, tag_reserve_c2},  {0,0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt3));
    
    // (C1) change c1sblk
    int beg = bmm + 600;
    tvars.c1_sblk.n = 10;
    tvars.beginposmm = 0+beg;
    tvars._curposmm = 0+beg;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_mm == 20);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt4 = {0, 0, 1, 0, 5, { {beg+20, tag_chkocc},  {beg+300,tag_stop_eot}, {beg+140, tag_brake}, {beg+400, tag_end_lsblk}, {beg+300, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt4));

    // (D1) advance to first trig
    tvars._curposmm += 20;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_mm == 100);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    const rettrigs_t expt5 = {0, 0, 1, 0, 5, { {beg+100+20, tag_chkocc},  {beg+300,tag_stop_eot}, {beg+140, tag_brake}, {beg+400, tag_end_lsblk}, {beg+300, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt5));

    

    // (C2) identical to C1, but reset beginposmm
    tvars.c1_sblk.n = 10;
    tvars.beginposmm = 0;
    tvars._curposmm = 0;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_mm == 20);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt4b = {0, 0, 1, 0, 5, { {20, tag_chkocc},    {300,tag_stop_eot}, {140, tag_brake}, {400, tag_end_lsblk}, {300, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt4b));
    
    
    
}

#if 0

- (void) testProgressRight
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    topology_set_turnout(to1, 1, -1);
    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 600;             //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 27);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, { {87, tag_chkocc}, {0, 0}, {0,0}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
    
    // ------------------------
    // trains progress until triggering of tag_chkocc
    tvars._curposmm = rettrigs.trigs[0].poscm*10; // 870
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 90-87+45-48+54); // 54

    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt2 = {0, 0, { {0, 0}, {0, 0}, {0,0}}};
    // no trig until c1 changes
    XCTAssert(!memcmp(&rettrigs, &expt2, sizeof(rettrigs_t)));
    
    // change c1
    // ----- b1 (45cm) ------+---- b3 (54cm) ----|end
    // <-----------(48)-----------> rlen = 54-3

    tvars.c1_sblk.n = 1;
    tvars._curposmm = 0;
    tvars.beginposmm = 0;
    rc = ctrl3_update_front_sblks_c1changed(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 51); // 27
    
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt5 = {0, 0, { {0, 0}, {23, tag_brake}, {39, tag_stop_eot}}};
    XCTAssert(!memcmp(&rettrigs, &expt5, sizeof(rettrigs_t)));
    
    // up to first trig
    tvars._curposmm = rettrigs.trigs[1].poscm*10; // 230
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 28); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==16);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt6 = {0, 0, { {0, 0}, {0, 0}, {39, tag_stop_eot}}};
    XCTAssert(!memcmp(&rettrigs, &expt6, sizeof(rettrigs_t)));

    // up to last trig
    tvars._curposmm = rettrigs.trigs[2].poscm*10; // 390
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 12); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc<0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt7 = {1, 0, { {0, 0}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt7, sizeof(rettrigs_t)));
}


- (void) testStartNoOccTurnout
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 1, -1);
    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 620;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 25);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, { {87, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
}
- (void) testStartOccTurnout
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 0, -1);
    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 620;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 25);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==13);
    const rettrigs_t expt1 = {0, 0, { {87, tag_chkocc}, {0, 0}, {75, tag_stop_blk_wait}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
}



- (void) testStartNoOccTurnout2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 1, -1);
    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 750;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 12);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, { {87, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
}
- (void) testStartOccTurnout2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 0, -1);
    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 750;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 12);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==-1);
    const rettrigs_t expt1 = {0, 1, { {87, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
}

- (void) testProgressRight2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, 1, -1);
    // same than previous, but to1 disallowed going forward
    topology_set_turnout(to1, 0, -1);
    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 600;             //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.nr == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_cm == 27);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==15);  // 27 cm rlen -> 12 margin + 16 brake
    const rettrigs_t expt1 = {0, 0, { {87, tag_chkocc}, {0, 0}, {75, tag_stop_blk_wait}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
    
    // ------------------------
    // trains progress 2cm
    tvars._curposmm = 620; // (rettrigs[2].poscm-13)*10; // 602
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==13);
    // no change in trig
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));

    // go until end of brake (+15cm)
    tvars._curposmm = 600+150; // (rettrigs[2].poscm-13)*10; // 602
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==-1);
    const rettrigs_t expt2 = {0, 1, { {87, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt2, sizeof(rettrigs_t)));

    // change to1 to normal
    topology_set_turnout(to1, 1, -1);
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt3 = {0, 0, { {87, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt3, sizeof(rettrigs_t)));
    
}

// ----------------------------------------------------------
// ----------------------------------------------------------

- (void) testLeft1
{
    tconf->trainlen_left_cm = 19;
    tconf->trainlen_right_cm = 0;
    tvars._curposmm = 30;
    lsblk_num_t r[4] = {0};
    // (s0) - (s1)
    //         3cm     on s0, 19-3 = 16cm
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, NULL);
    XCTAssert(n==1);
    
    static const int exp1[] = { 0 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    
    
    tconf->trainlen_left_cm = 2;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, NULL);
    XCTAssert(n==0);
    
    
    
    tconf->trainlen_left_cm = 80;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, NULL);
    XCTAssert(n==1);
    static const int exp2[] = { 0};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
}



- (void) testProgressLeft
{
    tconf->trainlen_left_cm = 20;
    tconf->trainlen_right_cm = 8;
    occupency_clear();
    topology_set_turnout(to0, topo_tn_turn, -1);
    topology_set_turnout(to1, topo_tn_turn, -1);
    ctrl3_init_train(0, &tvars, sone, 1); // s1 45cm
    tvars._curposmm = (45-4)*10;
    // 20cm sur s1, reste 45-4-20 = 21
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.leftcars.nr == 0);
    //static const int exp2[] = { 2 };
    //int rc = check_lsblk_array(tvars.leftcars.r, exp2, 1);
    //XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_cm == 21);
    
        
    rettrigs_t rettrigs = {0};
    int rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, { {20, tag_chkocc}, {0, 0}, {0,0}}};
    XCTAssert(!memcmp(&rettrigs, &expt1, sizeof(rettrigs_t)));
    
    // ------------------------
    // trains progress until triggering of tag_chkocc
    tvars._curposmm = rettrigs.trigs[0].poscm*10; // 200
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_cm == 90); // s2 len
    
              //xxxx
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt2 = {0, 0, { {0, 0}, {0, 0}, {0,0}}};
    // no trig until c1 changes
    XCTAssert(!memcmp(&rettrigs, &expt2, sizeof(rettrigs_t)));
    
    // change c1
    // ----- b1 (45cm) ------+---- b3 (54cm) ----|end
    // <-----------(48)-----------> rlen = 54-3

    tvars.c1_sblk.n = 2;
    tvars._curposmm = 900; // s2 len = 90
    tvars.beginposmm = 0;
    rc = ctrl3_update_front_sblks_c1changed(0, &tvars, tconf, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_cm == 70); // 27
    
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    // rlen = 78 at start of s2
    // -> 70-12(margin) -> 58 stop  90-58 -> 32 (20+12)
    // -> 70-12(margin)-16(brake) -> 42 start braking 90-42 -> 48 (20+12+16)
    const rettrigs_t expt5 = {0, 0, { {20, tag_chkocc}, {48, tag_brake}, {32, tag_stop_eot}}};
    XCTAssert(!memcmp(&rettrigs, &expt5, sizeof(rettrigs_t)));
    
    // up to first trig
    tvars._curposmm = rettrigs.trigs[1].poscm*10; // 230
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_cm == 28); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==16);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt6 = {0, 0, { {20, tag_chkocc}, {0, 0}, {32, tag_stop_eot}}};
    XCTAssert(!memcmp(&rettrigs, &expt6, sizeof(rettrigs_t)));

    // up to last trig
    tvars._curposmm = rettrigs.trigs[2].poscm*10; // 320
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_cm == 12); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc<0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt7 = {1, 0, { {20, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!memcmp(&rettrigs, &expt7, sizeof(rettrigs_t)));
}

// ----------------------------------------------------------
// ----------------------------------------------------------

// ----------------------------------------------------------
// ----------------------------------------------------------




// ----------------------------------------------------------
// ----------------------------------------------------------




#endif
@end
