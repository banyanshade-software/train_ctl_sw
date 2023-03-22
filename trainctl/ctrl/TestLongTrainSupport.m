//
//  TestLongTrainSupport.m
//  trainLPctlTests
//
//  Created by Daniel Braun on 13/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//


#import <XCTest/XCTest.h>


#include "misc.h"
#include "topology.h"
#include "occupency.h"
#include "ctrlLT.h"
#include "longtrain.h"
#include "trig_tags.h"

#include "TestLongTrainSupport.h"


void c3auto_set_turnout(int tidx, xtrnaddr_t tn)
{
    abort();
}


int cmptrigs(const rettrigs_t *r1, const rettrigs_t *r2)
{
    if (r1->isocc != r2->isocc) return -1;
    if (r1->isoet != r2->isoet) return -2;
    if (r1->power_c2 != r2->power_c2) return -3;
    if (r1->res_c2 != r2->res_c2) return -4;
    if (r1->ntrig != r2->ntrig) return -5;
    for (int i=0; i<NUMTRIGS; i++) {
        if (r1->trigs[i].posmm != r2->trigs[i].posmm) {
            return i+10;
        }
        if (r1->trigs[i].tag != r2->trigs[i].tag) {
            return i+100;
        }
    }
    return 0;
}





int errorhandler = 0;
void Error_Handler(void)
{
    errorhandler++;
}
void dump_msg(mqf_t *mq, int n)
{
    errorhandler++;
}



// in unit tests, only needed for last_tick
static const tasklet_def_t ctrl_tdef = {0};
tasklet_t ctrl_tasklet = { .def = &ctrl_tdef, .init_done = 0, .queue=NULL};


void FatalError(const char *shortsmsg, const char *longmsg, enum fatal_error_code errcode)
{
    errorhandler++;
    //abort();
}
int ignore_ina_pres(void)
{
    return 0;
}
int ignore_bemf_pres(void)
{
    return 0;
}
uint32_t SimuTick = 0;
int tsktick_freqhz = 100;



static msg_64_t qbuf[16];

mqf_t from_ctrl =  {
    .head=0,
    .tail=0,
    .msgsiz=sizeof(msg_64_t),
    .num=16,
    .maxuse=0,
    .msgbuf=(uint8_t *) qbuf,
    .silentdrop=0
    
};

/*void trace_train_dump(int tidx)
{
}*/
void __trace_train_append_line(char *s)
{
}

int compareMsg64(const msg_64_t *exp, int n, int clear)
{
    int rc = 0;
    if (mqf_len(&from_ctrl) != n) {
        NSLog(@"expect %d msg, got %d", n, mqf_len(&from_ctrl));
        rc = -2;
    } else {
        for (int i=0; i<n; i++) {
            // per msg compare, for easier debug
            if (memcmp(&qbuf[i], &exp[i], sizeof(msg_64_t))) {
                NSLog(@"%d exp: %2.2x %2.2x cmd=%2.2x subc=%d v1=%d v2=%d", i,
                      exp[i].from, exp[i].to, exp[i].cmd, exp[i].subc, exp[i].v1, exp[i].v2);
                NSLog(@"%d got: %2.2x %2.2x cmd=%2.2x subc=%d v1=%d v2=%d", i,
                      qbuf[i].from, qbuf[i].to, qbuf[i].cmd, qbuf[i].subc, qbuf[i].v1, qbuf[i].v2);
                rc = i+1;
                break;
            }
        }
    }
    if (clear) {
        mqf_clear(&from_ctrl);
    }
    return rc;
}

int compareMsg64_itrig(const msg_64_t *exp, int n, int clear)
{
    int rc = 0;
    int nq = 0;
    int ql = mqf_len(&from_ctrl);
    for (int i=0; i<n; i++) {
        if (exp[i].cmd == CMD_POSE_SET_TRIG) continue;
        msg_64_t m = {0};
        for (;;) {
            if (nq>=ql) {
                NSLog(@"missing q msg");
                return -1;
            }
            m = qbuf[nq];
            nq++;
            if (m.cmd == CMD_POSE_SET_TRIG) continue;
            break;
        }
        if (memcmp(&m, &exp[i], sizeof(msg_64_t))) {
            NSLog(@"%d exp: %2.2x %2.2x cmd=%2.2x subc=%d v1=%d v2=%d", i,
                  exp[i].from, exp[i].to, exp[i].cmd, exp[i].subc, exp[i].v1, exp[i].v2);
            NSLog(@"%d got: %2.2x %2.2x cmd=%2.2x subc=%d v1=%d v2=%d", i,
                  m.from, m.to, m.cmd, m.subc, m.v1, m.v2);
            rc = i+1;
            break;
        }
    }
    if (nq != ql) {
        NSLog(@"too many q msg");
        return -1;
    }
    if (clear) {
        mqf_clear(&from_ctrl);
    }
    return rc;
}


NSString *dump_msgbuf(int clear)
{
    NSString *r = @"";
    int first = 1;
    for (int i=0; i<from_ctrl.head; i++) {
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
        mqf_clear(&from_ctrl);
    }
    return r;
}




void record_msg_read(void *ptr)
{
    
}

void c3auto_set_s1(int tidx, lsblk_num_t s1)
{
    
}
void c3auto_start(int tidx)
{
    
}
void c3auto_freeback(int tidx, lsblk_num_t freelsblk)
{
    
}
void c3auto_station(int tidx)
{
    
}
