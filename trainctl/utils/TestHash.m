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
#define NUM_VAL  40
#define MAX_COL  0
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

static int div_top(int a, int b)
{
    return (a+(b-1))/b;
}


/* Bit map related macros. */
typedef uint8_t bit128_t[16];
#define ba128_setbit(ba, i)     ((ba)[(i)/8] |= 1u<<((i)%8))
#define ba128_clrbit(ba, i)     ((ba)[(i)/8] &= ~(1u<<((i)%8)))
#define ba128_isset(ba, i)      ((ba)[(i)/8] & (1u<<((i)%8)))
#define ba128_isclr(ba, i)      (((ba)[(i)/8] & (1u<<((i)%8))) == 0)

static inline void ba128_and(bit128_t a, bit128_t b)
{
    for (int i=0; i<16; i++) a[i] &= b[i];
}

static inline void ba128_or(bit128_t a, bit128_t b)
{
    for (int i=0; i<16; i++) a[i] |= b[i];
}

static inline void ba128_clr(bit128_t a, bit128_t b)
{
    for (int i=0; i<16; i++) a[i] &= ~b[i];
}

static inline int ba128_empty(bit128_t a)
{
    for (int i=0; i<16; i++) {
        if (a[i]) return 0;
    }
    return 1;
}

static int test_dph(void)
{
    int ncoll = 0;
    memset(used, 0, sizeof(used));
    memset(coll, 0, sizeof(coll));
    if ((0)) {
        unsigned int tab[NUM_VAL] = {0};//{17, 138, 173, 294, 306, 472, 540, 551, 618}; //{0};
        if ((1)) {
            int j = 0;
            for (int i=0; i<256; i++) {
                if (!values[i]) continue;
                tab[j]=i;
                j++;
            }
            XCTAssert(j==NUM_VAL);
        }
        int N0 = 9999999;
        for (int i=0; i<NUM_VAL-1; i++) {
            for (int j = i+2; j<NUM_VAL; j++) {
                int v = (tab[j]-tab[i]-1)/((j+1)-(i+1)-1);
                if (v<N0) N0=v;
            }
        }
        printf("N0 %d\n", N0);
        XCTAssert(N0<128);
        
        // (b)
        bit128_t Delta  = {0};
        for (int k=0; k <= N0; k++) ba128_setbit(Delta, k);
        
        // (c)
        for (int i=0; i<NUM_VAL-1; i++) {
            for (int j=i+1; j<NUM_VAL-1; j++) {
                // if di+dj < N0
                int di = tab[i+1]-tab[i];
                int dj = tab[j+1]-tab[j];
                if (di+dj < N0) { // (c)
                    // (c1)
                    int d = di+dj-1;
                    // (c2)
                    int m = (tab[j]+1) - tab[i+1];
                    int M = tab[j+1] - (tab[i]+1);
                    int tp = div_top(m, N0); // m/N0;
                    int ts = div_top(M, d+1); // M/(d+1);
                    // (c3)
                    bit128_t D = {0};
                    for (int t = tp; t<=ts; t++) {
                        // D = D U [ m/t, M,t]
                        for (int k = div_top(m,t); k <= M/t; k++) {
                            ba128_setbit(D, k);
                            //D = D | (1<<k);
                        }
                        // Delta = Delta inter D
                        //Delta = Delta & D;
                        ba128_and(Delta, D);
                    }
                }
            }
        }
        int N;
    stepd:
        // (d)
        N = 0;
        for (int k=0; k<128; k++) {
            //if (Delta & (1<<k)) N = k;
            if (ba128_isset(Delta, k)) N = k;
        }
        XCTAssert(N>0);
        XCTAssert(N<128);
        // (e)
        bit128_t J = {0};
        for (int k=0; k<=N; k++) ba128_setbit(J, k); // J |= (1<<k);
        // (f)
        for (int i=0; i<NUM_VAL-1; i++) {
            int di = tab[i+1]-tab[i];
            if (di>=N) continue;
            // J <- J inter ... (X)
            bit128_t X = {0};
            for (int t = 0; t<di; t++) {
                int k = (t - tab[i+1]) % N;
                //X |= (1<<k);
                ba128_setbit(X, k);
            }
            ba128_and(J, X);
            //J = J & X;
        }
        // (g)
        if (1 && ba128_empty(J)) { // (J == 0) {
            ba128_clrbit(Delta, N);
            //Delta  &= ~(1<<N);
            goto stepd;
        }
        if (!N) N=1;
        //(h)
        int t = -1;
        int mv = 9999;
        for (int k = 0; k<32; k++) {
            if (!ba128_isset(J, k)) continue;
            //if (!(J&(1<<k))) continue;
            int v = (tab[0]+k) % N;
            if (v<mv) {
                t = k;
                mv = v;
                if (!v) break;
            }
        }
        // (i)
        int s = t - N*((tab[0]+t)/N);
        //
        printf("N=%d s=%d\n", N, s);
        
        dph.p0 = N;
        dph.p1 = s;
        
    }
    
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
        if (rc<=MAX_COL) break;
        if ((0)) {
            printf("col %d\n", rc);
            return rc;
        }
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
    int numvalues = NUM_VAL;
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
