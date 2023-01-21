//
//  TestServo.m
//  train_lowTests
//
//  Created by Daniel Braun on 06/11/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>

#include "misc.h"
//#include "topology.h"
//#include "occupency.h"
//#include "ctrlP.h"
#include "../low/servo.h"


#ifndef UNIT_TEST
#error UNIT_TEST  should be defined for unit tests
#endif
#ifndef TRN_BOARD_SIMU
//#error TRN_BOARD_SIMU not defined, expect bad config
#endif


@interface TestServo : XCTestCase

@end

uint32_t SimuTick = 0;

static int errorhandler = 0;
void Error_Handler(void)
{
    errorhandler++;
}
void dump_msg(mqf_t *mq, int n)
{
    errorhandler++;
}

static NSString *dump_msgbuf(int clear);
static int compareMsg64(const msg_64_t *exp, int n, int clear);




static msg_64_t qbuf[16];
static msg_64_t xbuf[16];

mqf_t from_servo =  {
    .head=0,
    .tail=0,
    .msgsiz=sizeof(msg_64_t),
    .num=16,
    .maxuse=0,
    .msgbuf=(uint8_t *) qbuf,
    .silentdrop=0
};

mqf_t to_servo =  {
    .head=0,
    .tail=0,
    .msgsiz=sizeof(msg_64_t),
    .num=16,
    .maxuse=0,
    .msgbuf=(uint8_t *) xbuf,
    .silentdrop=0
};

static NSString *dump_msgbuf(int clear)
{
    NSString *r = @"";
    int first = 1;
    for (int i=0; i<from_servo.head; i++) {
        r = [r stringByAppendingFormat:@"%s{%2.2X, %2.2X, %2.2X, %d, %d}",
             first ? "" : ",",
             qbuf[i].from,
             qbuf[i].to,
             qbuf[i].cmd,
             qbuf[i].v1,
             qbuf[i].v2 ];
        first = 0;
    }
    if (clear) {
        mqf_clear(&from_servo);
    }
    return r;
}


void FatalError(const char *shortsmsg, const char *longmsg, enum fatal_error_code errcode)
{
    abort();
}


static int compareMsg64(const msg_64_t *exp, int n, int clear)
{
    int rc = 0;
    if (mqf_len(&from_servo) != n) {
        rc = -2;
    } else {
        for (int i=0; i<n; i++) {
            // per msg compare, for easier debug
            if (memcmp(&qbuf[i], &exp[i], sizeof(msg_64_t))) {
                NSLog(@"%d exp: %2.2x %2.2x cmd=%2.2x v1=%d v2=%d", i,
                      exp[i].from, exp[i].to, exp[i].cmd, exp[i].v1, exp[i].v2);
                NSLog(@"%d got: %2.2x %2.2x cmd=%2.2x v1=%d v2=%d", i,
                      qbuf[i].from, qbuf[i].to, qbuf[i].cmd, qbuf[i].v1, qbuf[i].v2);
                rc = i+1;
                break;
            }
        }
    }
    if (clear) {
        mqf_clear(&from_servo);
    }
    return rc;
}

int oam_localBoardNum(void)
{
    return 1;
}


// -----------------------------------

#define EXPMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    printf("n=%d",n);                                        \
    int rc = compareMsg64(exp, n, 1);                        \
    XCTAssert(!rc);                                          \
} while (0)


@implementation TestServo

- (void)setUp {
    errorhandler = 0;
    mqf_clear(&from_servo);
    mqf_clear(&to_servo);

    servo_tasklet.def->init();

    //tvars._mode = train_manual;
   // ctrl2_init_train(0, &tvars, sone);
    NSLog(@"init done");
}

- (void)tearDown {
    XCTAssert(0==errorhandler, "Error_Handler");
}

- (void)testServo1
{
    msg_64_t m = {0};
    m.from = MA3_UI_CTC;
    m.to = MA0_SERVO(oam_localBoardNum());
    m.cmd = CMD_SERVO_SET;
    m.subc = 0;
    m.v1u = 2200;
    m.v2u = 0;
    mqf_write(&to_servo, &m);
    for (int i = 0; i<1000; i++) {
        SimuTick += 10;
        tasklet_run(&servo_tasklet, SimuTick);
    }
    NSString *s = dump_msgbuf(0);
    // UT config is 2300-7230
    EXPMSG({.to=MA3_UI_CTC,   .from=MA0_SERVO(1), .subc=0, .cmd=CMD_SERVO_ACK, .v1u=2300});
}


- (void)testServo2
{
    msg_64_t m = {0};
    m.from = MA3_UI_CTC;
    m.to = MA0_SERVO(oam_localBoardNum());
    m.cmd = CMD_SERVO_SET;
    m.subc = 0;
    m.v1u = 4000;
    m.v2u = 0;
    mqf_write(&to_servo, &m);
    for (int i = 0; i<1000; i++) {
        SimuTick += 10;
        tasklet_run(&servo_tasklet, SimuTick);
    }
    NSString *s = dump_msgbuf(0);(void)s;
    EXPMSG({.to=MA3_UI_CTC,   .from=MA0_SERVO(1), .subc=0, .cmd=CMD_SERVO_ACK, .v1u=4000});
}


- (void)testServo3
{
    msg_64_t m = {0};
    m.from = MA3_UI_CTC;
    m.to = MA0_SERVO(oam_localBoardNum());
    m.cmd = CMD_SERVO_SET;
    m.subc = 0;
    m.v1u = 9000;
    m.v2u = 0;
    mqf_write(&to_servo, &m);
    for (int i = 0; i<1000; i++) {
        SimuTick += 10;
        tasklet_run(&servo_tasklet, SimuTick);
    }
    NSString *s = dump_msgbuf(0); (void) s;
    EXPMSG({.to=MA3_UI_CTC,   .from=MA0_SERVO(1), .subc=0, .cmd=CMD_SERVO_ACK, .v1u=7230});
}

@end


void record_msg_read(void *ptr)
{
    
}

