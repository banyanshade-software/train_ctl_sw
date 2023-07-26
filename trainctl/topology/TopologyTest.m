//
//  TopologyTest.m
//  train_throttle
//
//  Created by Daniel BRAUN on 27/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "topology.h"

#include "TestLongTrainSupport.h"

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
    xblkaddr_t caff = { .v = 0xFF };
    lsblk_num_t b = any_lsblk_with_canton(caff);
    XCTAssert(b.n==-1);
    lsblk_num_t bn = next_lsblk(b, 0, NULL);
    XCTAssert(bn.n==-1);
    lsblk_num_t b1 = next_lsblk(b, 1, NULL);
    XCTAssert(b1.n==-1);
    bn.n = 0;
    lsblk_num_t be = first_lsblk_with_canton(caff, bn);
    XCTAssert(be.n==-1);
}

- (void) testIna
{
    // ina 3 is on c5,  lsblk 12
    ina_num_t ina;
    ina.v = 3;
    lsblk_num_t ns = get_lsblk_for_ina(ina);
    XCTAssert(ns.n == 12);
    xblkaddr_t b = get_canton_for_ina(ina);
    XCTAssert(b.v == 5);
    lsblk_num_t l;
    xblkaddr_t c;
    get_lsblk_and_canton_for_ina(ina, &l, &c);
    XCTAssert(l.n == 12);
    XCTAssert(c.v == 5);
}

- (void) testIna2
{
    ina_num_t ina;
    ina.v = 4;
    // ina 4 is on c6, lsblk 13 to 22
    lsblk_num_t ns = get_lsblk_for_ina(ina);
    XCTAssert(ns.n == 13);
    xblkaddr_t b = get_canton_for_ina(ina);
    XCTAssert(b.v == 6);
    lsblk_num_t l;
    xblkaddr_t c;
    get_lsblk_and_canton_for_ina(ina, &l, &c);
    XCTAssert(l.n == 13);
    XCTAssert(c.v == 6);
}



- (void) testIna3
{

    uint16_t b = get_ina_bitfield_for_canton(6);
    XCTAssert(b == (1U<<4));
    
    
    b = get_ina_bitfield_for_canton(4);
    XCTAssert(b == ((1U<<1) | (1U<<2)));
    
    
}
@end
