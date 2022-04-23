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
#include "../config/conf_utestloc.h"
#include "../config/conf_utestloc.propag.h"

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

- (void)testSto1 {
    int32_t v00 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 0, 0);
    int32_t v01 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 0, 1);
    int32_t v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0);
    int32_t v11 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 1);
    XCTAssert(v00==1001);
    XCTAssert(v01==1001);
    XCTAssert(v10==1001);
    XCTAssert(v11==1001);

    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0, 42);
    v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0);
    XCTAssert(v10 == 42);
    
    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 1, 43);
    v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0);
    v11 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 1);
    XCTAssert(v10 == 42);
    XCTAssert(v11 == 43);

    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0, 44);
    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0, 42);

    v10 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 0);
    XCTAssert(v10 == 42);
}


- (void)testSto2 {
   

    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 3, 421);
    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 4, 21);
    oam_flashstore_set_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 3, 22);
    
    int32_t v13 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 3);
    XCTAssert(v13==22);
    int32_t v14 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 4);
    XCTAssert(v14==21);

    oam_flash_deinit();
    oam_flash_init();

    v13 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 3);
    XCTAssert(v13==22);
    v14 = oam_flashstore_get_value(conf_pnum_utest, conf_numfield_utest_alpha, 1, 4);
    XCTAssert(v14==21);

}

- (void) testLoc1
{
    const conf_utestloc_t *loc0 = conf_utestloc_get(0);
    const conf_utestloc_t *loc1 = conf_utestloc_get(1);
    const conf_utestloc_t *loc2 = conf_utestloc_get(2);
    XCTAssert(loc0->fixed == 42);
    XCTAssert(loc2->fixed == 42);
    XCTAssert(loc0->alpha == 1000);
    XCTAssert(loc1->alpha == 1001);
    XCTAssert(loc2->alpha == 1002);
    int32_t v = oam_flashlocal_get_value(conf_lnum_utestloc, conf_numfield_utestloc_alpha, 1);
    XCTAssert(v == 1001);
    oam_flashlocal_set_value(conf_lnum_utestloc, conf_numfield_utest_alpha, 1, 3333);
    v = oam_flashlocal_get_value(conf_lnum_utestloc, conf_numfield_utestloc_alpha, 1);
    XCTAssert(v == 3333);
    XCTAssert(loc0->alpha == 1000);
    XCTAssert(loc2->alpha == 1002);
    XCTAssert(loc1->alpha == 3333);
    ((conf_utestloc_t *)loc0)->fixed = 111;
    //((conf_utestloc_t *)loc0)->alpha = 10;
    oam_flashlocal_commit(conf_lnum_utestloc);
    
    oam_flash_deinit();
    oam_flash_init();
    oam_flashlocal_read(conf_lnum_utestloc);
    loc0 = conf_utestloc_get(0);
    loc1 = conf_utestloc_get(1);
    loc2 = conf_utestloc_get(2);
    
    XCTAssert(loc0->fixed == 111);
    XCTAssert(loc0->alpha == 1000);
    XCTAssert(loc2->alpha == 1002);
    XCTAssert(loc1->alpha == 3333);
}



- (void) testLoc2
{
    const conf_utestloc_t *loc0 = conf_utestloc_get(0);
    const conf_utestloc_t *loc1 = conf_utestloc_get(1);
    const conf_utestloc_t *loc2 = conf_utestloc_get(2);
    //XCTAssert(loc0->fixed == 42);
    int32_t v;
    
    oam_flashlocal_set_value(conf_lnum_utestloc, conf_numfield_utest_alpha, 1, 5555);
    v = oam_flashlocal_get_value(conf_lnum_utestloc, conf_numfield_utestloc_alpha, 1);
    XCTAssert(v == 5555);
    XCTAssert(loc1->alpha == 5555);

    oam_flashlocal_commit(conf_lnum_utestloc);
    
    oam_flashlocal_set_value(conf_lnum_utestloc, conf_numfield_utest_alpha, 1, 3334);
    oam_flashlocal_commit(conf_lnum_utestloc);
    oam_flashlocal_set_value(conf_lnum_utestloc, conf_numfield_utest_alpha, 1, 3335);
    oam_flashlocal_commit(conf_lnum_utestloc);
    oam_flashlocal_set_value(conf_lnum_utestloc, conf_numfield_utest_alpha, 1, 3336);
    oam_flashlocal_commit(conf_lnum_utestloc);
    oam_flashlocal_set_value(conf_lnum_utestloc, conf_numfield_utest_alpha, 1, 3337);
    oam_flashlocal_commit(conf_lnum_utestloc);


    
    oam_flash_deinit();
    oam_flash_init();
    oam_flashlocal_read(conf_lnum_utestloc);
    
    loc1 = conf_utestloc_get(1);
    
    XCTAssert(loc1->alpha == 3337);
}
@end
