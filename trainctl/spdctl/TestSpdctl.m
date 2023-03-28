//
//  TestSpdctl.m
//  trainSpdctlTests
//
//  Created by Daniel Braun on 17/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "spdctl.h"
#include "trig_tags.h"
#include "ctrlLT.h"
#include "longtrain4.h"
#include "TestLongTrainSupport.h"


volatile int oscillo_trigger_start = 0;
volatile int oscillo_enable = 0;
volatile int oscillo_canton_of_interest = 0;

static msg_64_t qbuf1[16];
static msg_64_t qbuf2[16];

mqf_t from_spdctl =  {
    .head=0,
    .tail=0,
    .msgsiz=sizeof(msg_64_t),
    .num=16,
    .maxuse=0,
    .msgbuf=(uint8_t *) qbuf1,
    .silentdrop=0
};

mqf_t to_spdctl =  {
    .head=0,
    .tail=0,
    .msgsiz=sizeof(msg_64_t),
    .num=16,
    .maxuse=0,
    .msgbuf=(uint8_t *) qbuf1,
    .silentdrop=0
};

static const conf_canton_t canton_template = {
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_1,
     .notif_bemf = 0,
     .reverse = 0,
     .reverse_bemf = 1,
  };
const conf_canton_t *conf_canton_template(void)
{
  return &canton_template;
}


@interface TestSpdctl : XCTestCase

@end

@implementation TestSpdctl

- (void)setUp {
    spdctl_tasklet.def->init();
}

- (void)tearDown {
    extern int  errorhandler;
    XCTAssert(errorhandler==0);
    
}

#define SENDMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    int rcc = sendMsg64(exp, n, &to_spdctl);                        \
    XCTAssert(!rcc);                                          \
} while (0)

/* // to be updated to SET_C4
- (void)test1
{
    
    SENDMSG({.from=MA1_CTRL(0), .to=MA1_SPDCTL(0), .cmd=CMD_SET_C1_C2, .vb0=1, .vb1=1, .vb2=0xFF, .vb3=0});
    tasklet_run(&spdctl_tasklet, 10);
    NSString *s = dump_msgbuf(0);
    NSLog(@"...%@", s);
}
 */


@end
