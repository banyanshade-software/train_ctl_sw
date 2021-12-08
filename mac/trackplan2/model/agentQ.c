//
//  agentQ.c
//  trackplan2
//
//  Created by Daniel BRAUN on 05/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "agent.h"
#include "agentQ.h"
#include "model.h"


static void agentq_init(void);
static void agentq_setparams(double _alpha, double _gamma, double _epsilon, double _noise);
static void q_param(double,double,double,double,double,double,double,double);
static void agentq_restart(void);
static int q_step(int *retstate);


agent_def_t q_agent = {
    .init =     agentq_init,
    .restart =  agentq_restart,
    .step =     q_step,
    .setparam = q_param,
};

static double *qmatrix = NULL;

static double q_alpha   = 0.0;
static double q_gamma   = 0.0;
static double q_epsilon = 0.0;
static double q_noise   = 0.0;

static int curstate = 0;
static int final_state = 0;

static int q_idx(int state, int action)
{
    int i = state * model_num_actions() + action;
    return i;
}

double rand01(void)
{
    float x = ((float)rand()/(float)(RAND_MAX));
    if (fabs(x)>1.0) abort();
    return x;
}

static void agentq_init(void)
{
    if (qmatrix) free(qmatrix);
    int ns = model_num_states();
    int na = model_num_actions();
    qmatrix = malloc(na*ns*sizeof(double));
    memset(qmatrix, 0, na*ns*sizeof(double));
    if ((0)) {
        for (int i=0; i<na*ns; i++) {
            qmatrix[i] = (rand01()-.5)/4.0;
        }
    }
    int fs = model_final_state();
    for (int a=0; a<na; a++) {
        qmatrix[q_idx(fs, a)] = 0.0;
    }
    
    
    agentq_restart();
}

static void agentq_setparams(double _alpha, double _gamma, double _epsilon, double _noise)
{
    q_alpha = _alpha;
    q_gamma = _gamma;
    q_epsilon = _epsilon;
    q_noise = _noise;
}

static void q_param(double _alpha, double _gamma, double _epsilon, double _noise, double  u1, double u2, double u3, double u4)
{
    agentq_setparams(_alpha, _gamma, _epsilon, _noise);
}

static void agentq_restart(void)
{
    curstate = model_initial_state();
    final_state = model_final_state();
}

static double q_maxa(int state, int *pact)
{
    int na = model_num_actions();
    double m=0;
    int first = 1;
    for (int a=0; a<na; a++) {
        double noise = rand01()*q_noise;
        // adding noise here allow random choice when same values
        double q = qmatrix[q_idx(state, a)]+noise;
        if (first || (q>=m)) {
            first = 0;
            m = q;
            if (pact) *pact = a;
        }
    }
    return m;
}

static int agent_action(int curstate)
{
    int na = model_num_actions();
    double r = rand01();
    if (r < q_epsilon) {
        int r = (rand() % (na-1))+1;
        return r;
    } else {
        int a = action_dont_move;
        q_maxa(curstate, &a);
        return a;
    }
}

static int q_step(int *retstate)
{
    int action = agent_action(curstate);
    if (action<0) abort();
    if (action>=model_num_actions()) abort();
    
    double reward = 0.0;
    int badmove = 0;
    int newstate = model_new_state(curstate, action, &reward, &badmove, NULL);
    //double reward = model_reward(newstate);
    
    if (newstate < 0) {
        newstate = model_new_state(curstate, action, &reward, &badmove, NULL);
        abort();
    }
    
    //int newidx = q_idx(newstate, action);
    //double v = q_alpha * (reward + q_gamma*(q_maxa(curstate, NULL)-qmatrix[newidx]));
    //qmatrix[newidx] += v;
    
    q_dump_state(curstate);

    if ((0) && badmove) {
        int curidx = q_idx(curstate, action);
        qmatrix[curidx] = -1.0;
    } else {
        int curidx = q_idx(curstate, action);
        double v = q_alpha * (reward + q_gamma*(q_maxa(newstate, NULL)-qmatrix[curidx]));
        qmatrix[curidx] += v;
    }
    printf(" act %d -> new %d (rew %2.2f bad=%d)\n\n", action, newstate, reward, badmove);
  
    curstate = newstate;
    if (retstate) *retstate = newstate;
    if (newstate==final_state) {
        return 1;
    }
    return 0;
}

void q_dump_state(int st)
{
    int p[4];
    model_positions_for_state(st, &p[0], &p[1], &p[2], &p[3]);
    
    printf("st %d (T0:%d T1:%d   T2:%d T3:%d)\n", st, p[0], p[1], p[2], p[3]);
    for (int a=0; a<model_num_actions(); a++) {
        int train;
        model_elem_action_t eact;
        model_actions_for_num(a, &train, &eact);
        int idx = q_idx(st, a);
        double q = qmatrix[idx];
        printf("  act%d T%d %s : %2.2f\n", a, train, model_descr_action(eact), q);
    }
}
