//
//  TestAdcMean.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 14/02/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "adc_mean.h"

@interface TestAdcMean : XCTestCase

@end

@implementation TestAdcMean {
    adc_mean_ctx_t amctx;
}

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    adc_mean_init(&amctx);
}

- (void) testNone
{
    uint16_t v = adc_mean_get_mean(&amctx);
    XCTAssert(0==v);
}

- (void) testZero
{
    adc_mean_add_value(&amctx, 0);
    uint16_t v = adc_mean_get_mean(&amctx);
    XCTAssert(0==v);
}

- (void) testReset
{
    adc_mean_add_value(&amctx, 23);
    adc_mean_add_value(&amctx, 50);
    uint16_t v = adc_mean_get_mean(&amctx);
    XCTAssert(v>23);
    XCTAssert(v<50);
    adc_mean_init(&amctx);
    v = adc_mean_get_mean(&amctx);
    XCTAssert(0==v);
}

- (void) test1
{
    uint16_t v = adc_mean_get_mean(&amctx);
    XCTAssert(0==v);
    
    adc_mean_add_value(&amctx, 24);
    v = adc_mean_get_mean(&amctx);
    XCTAssert(12==v);
    
    adc_mean_add_value(&amctx, 32);
    v = adc_mean_get_mean(&amctx);
    XCTAssert(22==v);
    
    adc_mean_add_value(&amctx, 0);
    v = adc_mean_get_mean(&amctx);
    XCTAssert(20==v);
    
    adc_mean_add_value(&amctx, 24);
    v = adc_mean_get_mean(&amctx);
    XCTAssert(22==v);
    
    adc_mean_add_value(&amctx, 0);
    adc_mean_add_value(&amctx, 0);
    adc_mean_add_value(&amctx, 0);
    v = adc_mean_get_mean(&amctx);
    XCTAssert(16==v);
}
- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

@end
