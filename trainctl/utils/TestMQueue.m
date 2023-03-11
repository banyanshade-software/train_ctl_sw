//
//  TestMQueue.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 23/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "lf_mqueue.h"

#include "TestLongTrainSupport.h"

//#include "itm_debug.h"

//static int errorhandler = 0;
/*
void Error_Handler(void)
{
    errorhandler++;
}

void dump_msg(mqf_t *mq, int n)
{
    errorhandler++;
}
 */


@interface TestMQueue : XCTestCase

@end

//LFMQUEUE_DEF_H(testQueue, uint32_t)

LFMQUEUE_DEF_C(testQueue, uint32_t, 8, 1)

@implementation TestMQueue

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    errorhandler = 0;
    mqf_clear(&testQueue);
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    XCTAssert(0==errorhandler, "Error_Handler");
    mqf_clear(&testQueue);
}

- (void)testOneElem {
    XCTAssert(0==mqf_len(&testQueue));
    uint32_t m = 4242;
    int rc = mqf_read(&testQueue, &m);
    XCTAssert(rc==-1, "should return -1");
    XCTAssert(m==4242, "m should be unchanged");
    rc = mqf_write(&testQueue, &m);
    XCTAssert(rc==0, "should return 0");
    XCTAssert(1==mqf_len(&testQueue));
    m = 0;
    rc = mqf_read(&testQueue, &m);
    XCTAssert(rc==0, "should return 0");
    XCTAssert(m==4242, "m should be 4242");
}

- (void)testFull {
    XCTAssert(0==mqf_len(&testQueue));
    for (int i=0; i<7; i++) {
        uint32_t m = 42000+i;
        int rc = mqf_write(&testQueue, &m);
        XCTAssert(rc==0, "should return 0");
        XCTAssert((i+1)==mqf_len(&testQueue));
    }
    uint32_t m = 99;
    int rc = mqf_write(&testQueue, &m);
    XCTAssert(rc==-1, "should return 0");
    XCTAssert(7==mqf_len(&testQueue));

    for (int i=0; i<7; i++) {
        uint32_t m = 444;
        int rc = mqf_read(&testQueue, &m);
        XCTAssert(rc==0, "should return 0");
        XCTAssert(7-i-1==mqf_len(&testQueue));
        XCTAssert(m==42000+i);
    }
    XCTAssert(0==mqf_len(&testQueue));
}

- (void)_test_rot:(int)rot
{
    XCTAssert(0==mqf_len(&testQueue));
    for (int i=0; i<100; i++) {
        uint32_t m = 42000+i;
        int rc = mqf_write(&testQueue, &m);
        XCTAssert(rc==0, "should return 0");
        if (i<rot) continue;
        rc = mqf_read(&testQueue, &m);
        XCTAssert(rc==0, "should return 0");
        XCTAssert(m==42000+i-rot);
    }
    XCTAssert(rot==mqf_len(&testQueue));
}

- (void)testRot5 {
    [self _test_rot:5];
}

- (void)testRot0 {
    [self _test_rot:0];
}

- (void)testRot6 {
    [self _test_rot:6];
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
