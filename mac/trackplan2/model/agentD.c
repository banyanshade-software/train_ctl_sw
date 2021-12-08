//
//  agentD.c
//  trackplan2
//
//  Created by Daniel BRAUN on 06/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "agent.h"
#include "model.h"
#include "agentD.h"



static void agentd_init(void);

static void agentd_restart(void);
static int d_step(int *retstate);


agent_def_t d_agent = {
    .init =     agentd_init,
    .restart =  agentd_restart,
    .step =     d_step,
    .setparam = NULL,
};


static uint16_t *statematrix = NULL;
static uint16_t *statestack = NULL;
static uint16_t _stackidx = 0;

static void queue_add(uint16_t state)
{
    if ((1)) {
        printf("push %d(%d)\n", state, statematrix[state]);
    }
    statestack[_stackidx++] = state;
}
static uint16_t queue_poplowest(void)
{
    if ((1)) {
        printf("q:" );
        for (int i=0; i<_stackidx; i++) {
            printf("  %d(%d),", statestack[i], statematrix[statestack[i]]);
        }
        printf("\n");
    }
    uint16_t min = 0xFFFF;
    int mini=-1;
    for (int i=0; i<_stackidx; i++) {
        uint16_t s = statestack[i];
        if (statematrix[s]<min) {
            min = statematrix[s];
            mini = i;
        }
    }
    if (mini<0) abort();
    uint16_t r = statestack[mini];
    if (_stackidx-mini-1>0) {
        memmove(&statestack[mini], &statestack[mini+1], sizeof(uint16_t)*(_stackidx-mini-1));
    }
    _stackidx--;
    if ((1)) {
        printf("dequeue i=%d : %d(%d)\n", mini, r, min);
        printf("q: ");
        for (int i=0; i<_stackidx; i++) {
            printf("  %d(%d),", statestack[i], statematrix[statestack[i]]);
        }
        printf("\n");
    }
    return r;
}

static int finalstate = 0;
static void agentd_init(void)
{
    if (statematrix) free(statematrix);
    if (statestack) free(statestack);
    
    int ns = model_num_states();
    finalstate = model_final_state();
    statematrix = malloc(sizeof(uint16_t)*ns);
    statestack = malloc(sizeof(uint16_t)*ns);
    memset(statematrix, 0, sizeof(uint16_t)*ns);
    memset(statestack, 0, sizeof(uint16_t)*ns);
    _stackidx = 0;
    int na = model_num_actions();
    
    for (int i=0; i<ns; i++) {
        statematrix[i] = 0xFFFF;
    }
    statematrix[finalstate] = 0;
    int curstate = finalstate;
    
    for (;;) {
        if ((1)) {
            int p[4];
            model_positions_for_state(curstate, &p[0], &p[1], &p[2], &p[3]);
            printf("st %d (T0:%d T1:%d   T2:%d T3:%d)\n", curstate, p[0], p[1], p[2], p[3]);
        }
        int vcur = statematrix[curstate];
        for (int a=0; a<na; a++) {
            int bad , col;
            double reward;
            int ns = model_new_state(curstate, a, &reward, &bad, &col);
           
            if (bad) continue;
            if (col) continue;
            if (ns==curstate) continue;
            printf("  st %d a%d -> %d\n", curstate, a, ns);
            int va = statematrix[ns];
            if (va == 0xFFFF) {
                //unexplored
                queue_add(ns);
            }
            if (va>vcur+1) statematrix[ns] = vcur+1;
        }
        if (!_stackidx) break;
        curstate = queue_poplowest();// statestack[--stackidx];
    }
}

static int start_state = 0;
static int cur_state = 0;

static void agentd_restart(void)
{
    if (finalstate != model_final_state()) agentd_init();
    start_state = model_initial_state();
    cur_state = start_state;
}
static int d_step(int *retstate)
{
    if (cur_state == finalstate) {
        *retstate = finalstate;
        return 1;
    }
    int na = model_num_actions();
    if ((1)) {
        int p[4];
        model_positions_for_state(cur_state, &p[0], &p[1], &p[2], &p[3]);
        printf("> st %d (%d) (T0:%d T1:%d   T2:%d T3:%d)\n", cur_state, statematrix[cur_state], p[0], p[1], p[2], p[3]);
    }
    uint16_t vmin = 0xFFFF;
    int smin=-1;
    int amin;
    for (int a=0; a<na; a++) {
        int bad; int col;
        int ns = model_new_state(cur_state, a, NULL, &bad, &col);
        if (bad) continue;
        if (col) continue;
        if (ns==cur_state) continue;
        int v = statematrix[ns];
        if ((1)) {
            int train; model_elem_action_t eact;
            model_actions_for_num(a, &train, &eact);
            printf("  act%d T%d %s  ---> s %d v=%d\n", a, train, model_descr_action(eact),ns, v);
        }
        if (v<vmin) {
            vmin = v;
            smin = ns;
            amin = a;
        }
    }
    if (vmin == 0xFFFF) abort();
    cur_state = smin;
    *retstate = smin;
    return 0;
}
