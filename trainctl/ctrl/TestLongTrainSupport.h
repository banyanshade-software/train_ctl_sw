//
//  TestLongTrainSupport.h
//  train_throttle
//
//  Created by Daniel Braun on 13/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef TestLongTrainSupport_h
#define TestLongTrainSupport_h

#include "trainmsg.h"
#include "misc.h"

#include "ctrl.h"
#include "topology.h"
#include "ctrlLT.h"
#include "longtrain.h"

int cmptrigs(const rettrigs_t *r1, const rettrigs_t *r2);

extern int errorhandler;



NSString *dump_msgbuf(int clear);
int compareMsg64(const msg_64_t *exp, int n, int clear);
int compareMsg64_itrig(const msg_64_t *exp, int n, int clear);



#define EXPMSG(...) do {                                     \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    int rcc = compareMsg64(exp, n, 1);                        \
    XCTAssert(!rcc);                                          \
} while (0)


#define EXPMSG_ITRIG(...) do {                               \
    const msg_64_t exp[] =  { __VA_ARGS__ } ;                \
    int n = sizeof(exp)/sizeof(msg_64_t);                    \
    int rcc = compareMsg64_itrig(exp, n, 1);                 \
    XCTAssert(!rcc);                                         \
} while (0)

#define EXPMSG_NONE() do {                                   \
    int rcc = compareMsg64(NULL, 0, 1);                      \
    XCTAssert(!rcc);                                         \
} while (0)

extern mqf_t from_ctrl;
extern mqf_t to_spdctl;
extern mqf_t from_spdctl;

void sendMsg64(msg_64_t *msg, int nummsg, mqf_t *queue);

#endif /* TestLongTrainSupport_h */
