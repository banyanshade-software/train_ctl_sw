//
//  TestFlashStore.m
//  flashStoreTests
//
//  Created by danielbraun on 21/04/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#include "oam_flash.h"
#include "../config/conf_utest.h"
#include "../config/conf_utest.propag.h"

@interface TestFlashStore : XCTestCase

@end

@implementation TestFlashStore

uint32_t SimuTick = 0;
static int errorcnt = 0;

void Error_Handler(void)
{
    errorcnt++;
    
}

- (void)setUp {
    errorcnt = 0;
    oam_flash_init();
    oam_flash_erase();
}

- (void)tearDown {
    XCTAssert(!errorcnt);
    oam_flash_deinit();
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)test1 {
    int32_t v00 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 0, 0);
    int32_t v01 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 0, 1);
    int32_t v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 1, 0);
    int32_t v11 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 1, 1);
    XCTAssert(v00==1001);
    XCTAssert(v01==1001);
    XCTAssert(v10==1001);
    XCTAssert(v11==1001);

    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_alpha, 1, 0, 42);
    v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 1, 0);
    XCTAssert(v10 == 42);
    
    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_alpha, 1, 1, 43);
    v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 1, 0);
    v11 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 1, 1);
    XCTAssert(v10 == 42);
    XCTAssert(v11 == 43);

    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_alpha, 1, 0, 44);
    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_alpha, 1, 0, 42);

    v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_alpha, 1, 0);
    XCTAssert(v10 == 42);
}


@end
