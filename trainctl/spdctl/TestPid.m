//
//  TestPid.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 23/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "misc.h"
#include "pidctl.h"

@interface TestPid : XCTestCase

@end

@implementation TestPid

pidctl_config_t conf;
pidctl_vars_t vars;

- (void)setUp {
    memset(&vars, 0, sizeof(vars));
    conf.kP = 500;
    conf.kI = 1200;
    conf.kD = -100;
    pidctl_reset(&conf, &vars);
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)test1
{
    pidctl_set_target(&conf, &vars, 50);
    XCTAssert(vars.target_v == 50);
    int32_t v1 = pidctl_value(&conf, &vars, 0);
    XCTAssert(v1==31);
    int32_t v2 = pidctl_value(&conf, &vars, 5);
    XCTAssert(v2==33);
    int32_t v3 = pidctl_value(&conf, &vars, 8);
    XCTAssert(v3==36);
    
    // should be symetric
    pidctl_reset(&conf, &vars);
    pidctl_set_target(&conf, &vars, -50);
    XCTAssert(vars.target_v == -50);
    int32_t v1n = pidctl_value(&conf, &vars, 0);
    XCTAssert(v1n==-v1);
    int32_t v2n = pidctl_value(&conf, &vars, -5);
    XCTAssert(v2n==-v2);
    int32_t v3n = pidctl_value(&conf, &vars, -8);
    XCTAssert(v3n==-v3);
    
    // should return greater values if far from target
    pidctl_reset(&conf, &vars);
    pidctl_set_target(&conf, &vars, 50);
    XCTAssert(vars.target_v == 50);
    int32_t v1m = pidctl_value(&conf, &vars, 20);
    XCTAssert(v1m<v1);
    
    pidctl_reset(&conf, &vars);
    pidctl_set_target(&conf, &vars, 50);
    XCTAssert(vars.target_v == 50);
    int32_t v1p = pidctl_value(&conf, &vars, -20);
    XCTAssert(v1p>v1);
}


- (void)test2
{
    pidctl_set_target(&conf, &vars, 50);
    XCTAssert(vars.target_v == 50);
    
    int32_t c = 0;
    int32_t cmax = 0;
    for (int i=0; i<50; i++) {
        c = pidctl_value(&conf, &vars, c);
        if (c>cmax) cmax=c;
    }
    XCTAssert(abs(c-50)<2);
    XCTAssert(cmax<51);
}

static float simMotor(float power, int reset)
{
#define DELAY_N 4
    static float spd[DELAY_N] = {0};
    if (reset) {
        for (int i=0; i<DELAY_N; i++) spd[i] = 0.0;
    }
    for (int i=DELAY_N-1; i>0; i--) {
        spd[i] = spd[i-1];
    }
    spd[0] = spd[0]*.7 + 0.3*power*2;
    return spd[DELAY_N-1];
}


- (void)test3
{
    pidctl_config_t cnf;
    cnf.kP = 500;
    cnf.kI = 1200;//1200;
    cnf.kD = 4000;
    pidctl_reset(&cnf, &vars);
    pidctl_set_target(&cnf, &vars, 50);
    XCTAssert(vars.target_v == 50);
    simMotor(0, 1);
    int32_t c = 0;
    int32_t cmax = 0;
    int32_t v;
    int32_t vmax = 0;
    for (int i=0; i<50; i++) {
        v = simMotor(c,0);
        if (v>vmax) vmax = v;
        c = pidctl_value(&cnf, &vars, v);
        if (c>cmax) cmax=c;
    }
    XCTAssert(abs(v-50)<2);
    XCTAssert(vmax<67);
}


@end
