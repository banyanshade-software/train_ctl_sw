//
//  TestOccupency.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 03/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "topology.h"
#include "occupency.h"

@interface TestOccupency : XCTestCase

@end

static const lsblk_num_t snone = {-1};
static const lsblk_num_t szero = {0};
static const lsblk_num_t sone = {1};
static const lsblk_num_t s42 = {42};

@implementation TestOccupency

- (void)setUp {
    occupency_clear();
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testDir {
    XCTAssert(BLK_OCC_LEFT==occupied(-1));
    XCTAssert(BLK_OCC_RIGHT==occupied(1));
    XCTAssert(BLK_OCC_STOP==occupied(0));
}

- (void) test1
{
    set_block_addr_occupency(3, BLK_OCC_STOP, 4, s42);
    XCTAssert(BLK_OCC_STOP == get_block_addr_occupency(3));
    XCTAssert(BLK_OCC_FREE == get_block_addr_occupency(2));
    XCTAssert(BLK_OCC_FREE == get_block_addr_occupency(4));
    
    set_block_addr_occupency(3, BLK_OCC_FREE, 4, snone);
    XCTAssert(get_block_addr_occupency(3)>=BLK_OCC_DELAY1);
    
    for (int i=0; i<20; i++) {
        check_block_delayed(i*100);
    }
    XCTAssert(BLK_OCC_FREE == get_block_addr_occupency(3));
    XCTAssert(BLK_OCC_FREE == get_block_addr_occupency(2));
    XCTAssert(BLK_OCC_FREE == get_block_addr_occupency(4));

}

- (void) test2
{
    set_block_addr_occupency(3, BLK_OCC_STOP, 4, s42);
    XCTAssert(occupency_block_is_free(2, 5));
    XCTAssert(occupency_block_is_free(4, 5));
    XCTAssert(!occupency_block_is_free(3, 5));
    XCTAssert(occupency_block_is_free(3, 4));
}
@end
