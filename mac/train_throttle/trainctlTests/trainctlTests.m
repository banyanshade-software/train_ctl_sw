//
//  trainctlTests.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface trainctlTests : XCTestCase

@end

@implementation trainctlTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testExample {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
}

/*
- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}
*/

void FatalError( const char *short4lettersmsg,  const char *longmsg,  int errcode)
{
    abort();
}
int ignore_ina_pres(void)
{
    return 0;
}
@end
