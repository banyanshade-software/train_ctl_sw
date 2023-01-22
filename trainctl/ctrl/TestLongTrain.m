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
#include "ctrlLT.h"
#include "longtrain.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"

@interface TestLongTrain : XCTestCase

@end

@implementation TestLongTrain {
    train_ctrl_t tvars;
    conf_train_t *tconf;
}

int cmptrigs(const rettrigs_t *r1, const rettrigs_t *r2);


//static lsblk_num_t snone = {-1};
static lsblk_num_t szero = {0};
static lsblk_num_t sone = {1};
static lsblk_num_t stwo = {2};

static const xtrnaddr_t to0 = { .v = 0};
static const xtrnaddr_t to1 = { .v = 1};


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
    topology_set_turnout(to0, topo_tn_straight, -1);
    topology_set_turnout(to1, topo_tn_turn, -1);


    tvars._mode = train_manual;
    ctrl3_init_train(0, &tvars, sone, 1);
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

- (void) test1
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 19;
    tvars._curposmm = 30;
    lsblk_num_t r[4] = {0};
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    tconf->trainlen_right_cm = 30;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==0);
    
    tconf->trainlen_right_cm = 44;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n=1);
    
    static const int exp1[] = { 3 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    
    tconf->trainlen_right_cm = 120;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, NULL);
    XCTAssert(n==2);
    static const int exp2[] = { 3, -1};
    rc = check_lsblk_array(r, exp2, n);
    XCTAssert(!rc);
}

- (void) test2
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 19;
    ctrl3_init_train(0, &tvars, szero, 1);
    tvars._curposmm = 900;
    int16_t remain = -1;
    lsblk_num_t r[4] = {0};

    // int ctrl2_get_next_sblks_(_UNUSED_ int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlen)
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==1);    // len left is 9
    static const int exp1[] = { 1 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    XCTAssert(remain==10*(98-90+45-19));
    
    tconf->trainlen_right_cm = 72;
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 0, r, 4, &remain);
    XCTAssert(n==2);
    static const int exp3[] = {1, 3 };
    rc = check_lsblk_array(r, exp3, n);
    XCTAssert(!rc);
    XCTAssert(remain == 10*(98-90+45+54-72));
}

- (void) test3
{
    tconf->trainlen_left_cm = 35;
    tconf->trainlen_right_cm = 19;
    tvars._curposmm = 400; // 40cm out of 45 for s1
    int16_t remain = -1;
    lsblk_num_t r[4] = {0};
    
    int n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, &remain);
    XCTAssert(n==0);
    XCTAssert(remain == 400-350);
    
    tvars._curposmm = 50; // 5cm out of 45 for s1
    n = ctrl3_get_next_sblks_(0, &tvars, tconf, 1, r, 4, &remain);
    XCTAssert(n==1);
    static const int exp1[] = { 0 };
    int rc = check_lsblk_array(r, exp1, n);
    XCTAssert(!rc);
    XCTAssert(remain==10*(98+5-35));
}


- (void) test_chk_front_right
{
    tconf->trainlen_left_cm = 0;
    tconf->trainlen_right_cm = 48;
    occupency_clear();
    topology_set_turnout(to0, topo_tn_turn, -1);
    topology_set_turnout(to1, topo_tn_turn, -1);

    ctrl3_init_train(0, &tvars, stwo, 1); // s2 90cm
    tvars._curposmm = 600;             //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(90-60+45-48)); // 27
    
    // train is on sblk2, cars on 3 and 1
    // (2)----(3)- - <tn0> - - (1) - - -<tn1>- -(5)-
    //      (0)  ----/          (6)------/
    
    // train is on b2 (30cm) + 18cm of b1,
    // 45-18 = 27
    
    // train can goes right because turnout 1 is in good position
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = { 0, 0,  0, 0, 3,{{870, tag_chkocc}, {670, tag_reserve_c2}, {700,tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
    
    
    topology_set_turnout(to1, 0, -1);
    
    memset(&tvars.rightcars, 0xFF, sizeof(tvars.rightcars));
    memset(&tvars.leftcars, 0xFF, sizeof(tvars.leftcars));
    ctrl3_get_next_sblks(0, &tvars, tconf);

    //   ---- reste 30cm sur s2 et 48-30=18 sur s1 (45cm)
    // s1: front cars 18><     27cm              >
    // s1: front cars 18><15 braking><margin 12cm> (brake distance=16)
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==150);
    const rettrigs_t expt2 = { 0, 0, 0, 0, 3, { {870, tag_chkocc}, {750,tag_stop_blk_wait}, {700, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt2));
    
    tvars._curposmm = 600+10;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_mm == 260);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc>0);
    XCTAssert(rc==140);
    const rettrigs_t expt3 = {0, 0, 0, 0, 3, { {870, tag_chkocc},  {750,tag_stop_blk_wait}, {700, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt3));
    
    tvars._curposmm = 600+160;
    ctrl3_get_next_sblks(0, &tvars, tconf);
    XCTAssert(tvars.rightcars.rlen_mm == 110);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc<0);
    const rettrigs_t expt4 = {0, 1, 0, 1, 1,{ {870, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt4));
    
   
}

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
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 270);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, 0, 0, 3, { {870, tag_chkocc}, {670, tag_reserve_c2}, {700,tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
    
    // ------------------------
    // trains progress until triggering of tag_chkocc
    tvars._curposmm = rettrigs.trigs[0].posmm; // 870
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 10*(90-87+45-48+54)); // 54

    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt2 = {0, 0, 0, 1, 0,{ {0, 0}, {0, 0}, {0,0}}};
    // no trig until c1 changes
    XCTAssert(!cmptrigs(&rettrigs, &expt2));
    
    // change c1
    // ----- b1 (45cm) ------+---- b3 (54cm) ----|end
    // <-----------(48)-----------> rlen = 54-3

    tvars.c1_sblk.n = 1;
    tvars._curposmm = 0;
    tvars.beginposmm = 0;
    rc = ctrl3_update_front_sblks_c1changed(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 510); // 27
    
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt5 = {0, 0, 0, 0,  3, {{390, tag_stop_eot}, {230, tag_brake}, {250, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt5));
    
    // up to first trig
    tvars._curposmm = rettrigs.trigs[1].posmm; // 230
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 280); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==160);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt6 = {0, 0, 0, 0, 1, {  {390, tag_stop_eot}, {250, tag_need_c2}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt6));

    // up to last trig
    tvars._curposmm = rettrigs.trigs[0].posmm; // 390
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 120); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc<0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt7 = {1, 0, 0, 1, 0, { {0, 0}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt7));
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
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 250);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, 0, 1, 2, { {870, tag_chkocc}, {670, tag_reserve_c2}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
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
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 250);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==130);
    const rettrigs_t expt1 = {0, 0, 0, 1, 2, { {870, tag_chkocc}, {750, tag_stop_blk_wait}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
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
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 120);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, 1, 1, 1, { {870, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
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
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 120);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==-1);
    const rettrigs_t expt1 = {0, 1, 0, 1, 1,{ {870, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
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
    XCTAssert(tvars.rightcars.numlsblk == 1);
    static const int exp2[] = { 1 };
    int rc = check_lsblk_array(tvars.rightcars.r, exp2, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.rightcars.rlen_mm == 270);
    
        
    rettrigs_t rettrigs = {0};
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==150);  // 27 cm rlen -> 12 margin + 16 brake
    const rettrigs_t expt1 = {0, 0, 0, 1, 2,{ {870, tag_chkocc},{750, tag_stop_blk_wait}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
    
    // ------------------------
    // trains progress 2cm
    tvars._curposmm = 620; // (rettrigs[2].poscm-13)*10; // 602
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==130);
    // no change in trig
    XCTAssert(!cmptrigs(&rettrigs, &expt1));

    // go until end of brake (+15cm)
    tvars._curposmm = 600+150; // (rettrigs[2].poscm-13)*10; // 602
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==-1);
    const rettrigs_t expt2 = {0, 1, 0, 1, 1, { {870, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt2));

    // change to1 to normal
    topology_set_turnout(to1, 1, -1);
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 0, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt3 = {0, 0, 1, 1, 1, { {870, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt3));
    
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
    XCTAssert(tvars.leftcars.numlsblk == 0);
    //static const int exp2[] = { 2 };
    //int rc = check_lsblk_array(tvars.leftcars.r, exp2, 1);
    //XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_mm == 210);
    
    /* c1 (len 450)
     pos  410
      ---|----<ccccccL>--|
                      ^410
          (210)  (200) (40)
     */
    rettrigs_t rettrigs = {0};
    int rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt1 = {0, 0, 0, 1, 2, { {200, tag_chkocc}, {400, tag_reserve_c2}, {0,0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt1));
    
    // ------------------------
    // trains progress until triggering of tag_chkocc
    tvars._curposmm = rettrigs.trigs[0].posmm; // 200
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_mm == 900); // s2 len
    
              //xxxx
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    const rettrigs_t expt2 = {0, 0, 0, 1, 0, { {0, 0}, {0, 0}, {0,0}}};
    // no trig until c1 changes
    XCTAssert(!cmptrigs(&rettrigs, &expt2));
    
    // change c1
    // ----- b1 (45cm) ------+---- b3 (54cm) ----|end
    // <-----------(48)-----------> rlen = 54-3

    tvars.c1_sblk.n = 2;
    tvars._curposmm = 900; // s2 len = 90
    tvars.beginposmm = 0;
    rc = ctrl3_update_front_sblks_c1changed(0, &tvars, tconf, 1);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_mm == 700); // 27
    
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==0);
    // rlen = 78 at start of s2
    // -> 70-12(margin) -> 58 stop  90-58 -> 32 (20+12)
    // -> 70-12(margin)-16(brake) -> 42 start braking 90-42 -> 48 (20+12+16)
    const rettrigs_t expt5 = {0, 0, 0, 0, 3, { {200, tag_chkocc}, {320, tag_stop_eot}, {480, tag_brake}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt5));
    
    // up to brake trig
    tvars._curposmm = rettrigs.trigs[2].posmm; // 230
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_mm == 280); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc==160);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt6 = {0, 0, 0, 0, 2, { {200, tag_chkocc}, {320, tag_stop_eot}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt6));

    // up to last trig
    tvars._curposmm = rettrigs.trigs[1].posmm; // 320
    rc = ctrl3_update_front_sblks(0, &tvars, tconf, 0);
    XCTAssert(!rc);
    XCTAssert(tvars.leftcars.rlen_mm == 120); // 16+12
    rc = ctrl3_check_front_sblks(0, &tvars, tconf, 1, &rettrigs);
    XCTAssert(rc<0);
    // rlen = 51 at start of c1
    // -> 51-12(margin) -> stop
    // -> 51-12(margin)-16(brake) -> 23 start braking
    const rettrigs_t expt7 = {1, 0, 0, 0, 1, { {200, tag_chkocc}, {0, 0}, {0, 0}}};
    XCTAssert(!cmptrigs(&rettrigs, &expt7));
}

// ----------------------------------------------------------
// ----------------------------------------------------------

// ----------------------------------------------------------
// ----------------------------------------------------------




// ----------------------------------------------------------
// ----------------------------------------------------------


/*
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

static NSString *dump_msgbuf(int clear);
static int compareMsg64(const msg_64_t *exp, int n, int clear);

#define EXPMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    int rcc = compareMsg64(exp, n, 1);                        \
    XCTAssert(!rcc);                                          \
} while (0)
 */



@end
