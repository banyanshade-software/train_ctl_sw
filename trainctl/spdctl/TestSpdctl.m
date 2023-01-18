//
//  TestSpdctl.m
//  trainSpdctlTests
//
//  Created by Daniel Braun on 17/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "spdctl.h"
#include "trig_tags.h"
#include "longtrain.h"
#include "TestLongTrainSupport.h"

@interface TestSpdctl : XCTestCase

@end

@implementation TestSpdctl

- (void)setUp {
    spdctl_tasklet.def->init();
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
    
}

#define SENDMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    int rcc = sendMsg64(exp, n, &to_spdctl);                        \
    XCTAssert(!rcc);                                          \
} while (0)

- (void)test1
{
    SENDMSG({.from=MA1_CTRL(0), .to=MA1_SPDCTL(0), .cmd=CMD_SET_C1_C2, .vb0=1, .vb1=1, .vb2=0xFF, .vb3=0});
    tasklet_run(&spdctl_tasklet, 10);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
}

- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}

@end
