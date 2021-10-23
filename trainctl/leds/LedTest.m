//
//  LedTest.m
//  trainctlTests
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#import <XCTest/XCTest.h>
#include "led.h"

@interface LedTest : XCTestCase

@end


static char outled0[1024*2];
static char *outled0ptr = NULL;
static int led_err = 0;

void led_io(uint8_t lednum, uint8_t v)
{
    if ((v!=0) && (v!=1)) {
        printf("bad v value\n");
        led_err++;
        return;
    }
    if (lednum != 0) {
        printf("testing only led 0\n");
        led_err++;
        return;
    }
    *outled0ptr++ = v ? '1':'0';
}

@implementation LedTest


- (void) commonCheck:(NSInteger)l
{
    XCTAssert(led_err==0, "led err");
    XCTAssert(strlen(outled0)==l, "bad len");
}

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    memset(outled0, '\0', sizeof(outled0)-1);
    outled0ptr = outled0;
    led_err = 0;
    
    led_reset_all();
    led_run_all();
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void)testLed25p {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    led_start_prog(0, LED_PRG_25p);
    for (int i=0; i<100; i++) {
        led_run_all();
    }
    [self commonCheck:100];
    for (int i=0; i<100; i++) {
        if ((i%4)==3) {
            XCTAssert(outled0[i]=='1', "0 expected");
        } else {
            XCTAssert(outled0[i]=='0', "0 expected");
        }
    }
}


- (void)testLed50p {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    led_start_prog(0, LED_PRG_50p);
    for (int i=0; i<100; i++) {
        led_run_all();
    }
    [self commonCheck:100];
    for (int i=0; i<100; i++) {
        if ((i%2)==1) {
            XCTAssert(outled0[i]=='1', "0 expected");
        } else {
            XCTAssert(outled0[i]=='0', "0 expected");
        }
    }
}

- (void)testLedOff {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    led_start_prog(0, LED_PRG_OFF);
    for (int i=0; i<10; i++) {
        led_run_all();
    }
    [self commonCheck:1];
    XCTAssert(!strcmp(outled0, "0"), "bad output");
}

- (void)testLedOn {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    led_start_prog(0, LED_PRG_ON);
    for (int i=0; i<10; i++) {
        led_run_all();
    }
    [self commonCheck:1];
    XCTAssert(!strcmp(outled0, "1"), "bad output");
}


const char *tstref =  "\
000100000000000110000011\
000100000000000110000011\
000100000000000110000011\
000000000011111111111111\
000000000011111111111111\
000000000011111111111111\
000000000011111111111111\
110000000000001000000000\
110000000000001000000000\
101010101010101010101010\
101010101010101010101010\
101010101010101010101010\
111111111111111111111111\
111111111111111111111111\
111111111111111111111111\
111111111111111111111111\
111111111111111111111111\
111111111111111111111111\
111111111111111111111111\
101010101010101011111110\
101010101010101011111110\
1";

static int ncmp(const char *a, const char *b)
{
    for (int i=0; ;i++) {
        if (a[i] != b[i]) return i;
        if (!a[i]) return i;
    }
    return -1;
}

- (void)testLedTest {
    // This is an example of a functional test case.
    // Use XCTAssert and related functions to verify your tests produce the correct results.
    led_start_prog(0, LED_PRG_TST);
    for (int i=0; i<1000; i++) {
        led_run_all();
    }
    [self commonCheck:strlen(tstref)];
    int k = ncmp(tstref, outled0);
    
    XCTAssert(!strcmp(outled0, tstref), "bad output");
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
