//
//  TopologyTest.m
//  train_throttle
//
//  Created by Daniel BRAUN on 27/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "topology.h"

@interface TopologyTest : XCTestCase

@end

@implementation TopologyTest

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testAnyFF
{
    lsblk_num_t b = any_lsblk_with_canton(0xFF);
    XCTAssert(b.n==-1);
    lsblk_num_t bn = next_lsblk(b, 0, NULL);
    XCTAssert(bn.n==-1);
    lsblk_num_t b1 = next_lsblk(b, 1, NULL);
    XCTAssert(b1.n==-1);
    bn.n = 0;
    lsblk_num_t be = first_lsblk_with_canton(0xFF, bn);
    XCTAssert(be.n==-1);
}


@end
