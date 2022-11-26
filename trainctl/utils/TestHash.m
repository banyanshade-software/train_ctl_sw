//
//  TestHash.m
//  trainctlTests
//
//  Created by Daniel Braun on 11/11/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "dynamic_perfect_hash.h"


@interface TestHash : XCTestCase

@end

@implementation TestHash

static uint8_t values[256];
#define NUM_HASH 64
static int used[NUM_HASH];
static int coll[NUM_HASH];

static dph64_def_t dph;
- (void)setUp {
   
}

- (void)tearDown {
  
}

static void alloc_64values(int numvalues)
{
    memset(values, 0, sizeof(values));
    for (int i=0; i<numvalues; i++) {
        for (;;) {
            int n = rand()%256;
            if (values[n]) continue;
            values[n] = 1;
            break;
        }
    }
}

static int test_dph(void)
{
    int ncoll = 0;
    memset(used, 0, sizeof(used));
    memset(coll, 0, sizeof(coll));
    for (int i=0; i<256; i++) {
        if (!values[i]) continue;
        uint8_t h = dph64_hash(&dph, i, NUM_HASH);
        if (h>=NUM_HASH) {
            printf("hash function not ok");
            abort();
            return -2;
        }
        if (used[h]) {
            //printf("  collision on i=%d h=%d u=%d\n", i, h, used[h]-1000);
            ncoll++;
            coll[h]++;
        } else {
            used[h] = 1000+i;
        }
    }
    return ncoll;
}
static int find_dph(void)
{
    dph64_start(&dph);
    int rc;
    int minncoll = 99;
    for (;;) {
        //printf("test hash %d %d %d\n", dph.p0, dph.p1, dph.p2);
        rc = test_dph();
        if (rc>=0 && rc<minncoll) minncoll = rc;
        if (!rc) break;
        if (rc<=2) break;
        int rc2 = dph64_next(&dph);
        if (rc2) {
            printf("  min ncoll: %d\n", minncoll);
            if (!minncoll) abort();
            return minncoll;
        }
    }
    printf("test hash ok %d %d %d %d  ncol=%d\n", dph.p0, dph.p1, dph.p2, dph.p3, rc);
    return 0;
}

- (void)testDph
{
    int bad = 0;
    int ntest = 100;
    int numvalues = 50;
    int maxncoll = 0;
    for (int i = 0; i<ntest; i++) {
        printf("-------- test %d\n", i);
        alloc_64values(numvalues);
        int rc = find_dph();
        if (rc) {
            printf("   hash not found\n");
            bad++;
        }
        if (rc>maxncoll) maxncoll = rc;
    }
    printf("bad : %d on %d, maxncoll=%d\n", bad, ntest, maxncoll);
    XCTAssert(0==bad);
}


@end
