//
//  TestInertia.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 23/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "misc.h"
//#define _UNUSED_
//#define DBG_INERTIA 0
//#define itm_debug2(_f, _t, _p1, _p2) do {} while(0)
#include "inertia.h"

int tsktick_freqhz = 100;
uint32_t SimuTick = 0;

@interface TestInertia : XCTestCase

@end

static inertia_config_t cnf = {200, 300};
static inertia_vars_t vars;

@implementation TestInertia

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    memset(&vars, 0, sizeof(vars));
    tsktick_freqhz = 100;
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testAcc1 {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    vars.target = 50;
    vars.cur100 = 0;
    cnf.acc = 300;
    cnf.dec = 50;
    int chg=0;
    int16_t v = vars.cur100/100;
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        v = v2;
    }
    XCTAssert(v == 30);
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        XCTAssert(v2<=50, "bad");
        v = v2;
    }
    XCTAssert(v == 50);
    for (int i=0; i<10; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        XCTAssert(v==v2, "should not change anymore");
        XCTAssert(!chg, "should not change anymore");
    }
}

- (void)testDec1 {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    vars.target = 5;
    vars.cur100 = 50*100;
    cnf.acc = 300;
    cnf.dec = 200;
    int chg=0;
    int16_t v = vars.cur100/100;
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2<=v, "bad");
        v = v2;
    }
    XCTAssert(v == 30);
    for (int i=0; i<300; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2<=v, "bad");
        XCTAssert(v2>=5, "bad");
        v = v2;
    }
    XCTAssert(v == 5);
    for (int i=0; i<10; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        XCTAssert(v==v2, "should not change anymore");
        XCTAssert(!chg, "should not change anymore");
    }
}


- (void)testAccNeg {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    vars.target = -50;
    vars.cur100 = 0;
    cnf.acc = 300;
    cnf.dec = 50;
    int chg=0;
    int16_t v = vars.cur100/100;
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2<=v, "bad");
        v = v2;
    }
    XCTAssert(v == -30);
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2<=v, "bad");
        XCTAssert(v2>=-50, "bad");
        v = v2;
    }
    XCTAssert(v == -50);
    for (int i=0; i<10; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        XCTAssert(v==v2, "should not change anymore");
        XCTAssert(!chg, "should not change anymore");
    }
}

- (void)testDecNeg {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    vars.target = -5;
    vars.cur100 = -50*100;
    cnf.acc = 300;
    cnf.dec = 100;
    int chg=0;
    int16_t v = vars.cur100/100;
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        v = v2;
    }
    XCTAssert(v == -40);
    for (int i=0; i<400; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        XCTAssert(v2<=-5, "bad");
        v = v2;
    }
    XCTAssert(v == -5);
    for (int i=0; i<90; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        XCTAssert(v==v2, "should not change anymore");
        XCTAssert(!chg, "should not change anymore");
    }
}

- (void)testAcc2 {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    vars.target = 50;
    vars.cur100 = -30*100;
    cnf.acc = 300;
    cnf.dec = 100;
    int chg=0;
    int16_t v = vars.cur100/100;
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        v = v2;
    }
    XCTAssert(v == -20);
    for (int i=0; i<200; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        XCTAssert(v2<=50, "bad");
        v = v2;
    }
    XCTAssert(v == 0);
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        XCTAssert(v2<=50, "bad");
        v = v2;
    }
    XCTAssert(v == 30);
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2>=v, "bad");
        XCTAssert(v2<=50, "bad");
        v = v2;
    }
    XCTAssert(v == 50);
    for (int i=0; i<10; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        XCTAssert(v==v2, "should not change anymore");
        XCTAssert(!chg, "should not change anymore");
    }
}



- (void)testDec2 {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    vars.target = 5;
    vars.cur100 = 50*100;
    cnf.acc = 300;
    cnf.dec = 150;
    int chg=0;
    int16_t v = vars.cur100/100;
    for (int i=0; i<100; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2<=v, "bad");
        v = v2;
    }
    XCTAssert(v == 35);
    for (int i=0; i<300; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        if (v != v2) XCTAssert(chg, "chg");
        else XCTAssert(!chg, "chg");
        XCTAssert(v2<=v, "bad");
        XCTAssert(v2>=5, "bad");
        v = v2;
    }
    XCTAssert(v == 5);
    for (int i=0; i<10; i++) {
        int16_t v2 = inertia_value(0, &cnf, &vars, &chg);
        XCTAssert(v==v2, "should not change anymore");
        XCTAssert(!chg, "should not change anymore");
    }
}

/*
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
*/
@end
