//
//  agentQ.c
//  trackplan2
//
//  Created by Daniel BRAUN on 05/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "agentQ.h"
#include "model.h"

static double *qmatrix = NULL;

static double q_alpha = 0.0;
static double q_gamma = 0.0;
static double q_epsilon = 0.0;
static double q_noise = 0.0;

static int curstate = 0;


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

void agentq_init(void)
{
    if (qmatrix) free(qmatrix);
    int ns = model_num_states();
    int na = model_num_actions();
    qmatrix = malloc(na*ns*sizeof(double));
    memset(qmatrix, 0, na*ns*sizeof(double));
    if ((1)) {
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

void agentq_setparams(double _alpha, double _gamma, double _epsilon, double _noise)
{
    q_alpha = _alpha;
    q_gamma = _gamma;
    q_epsilon = _epsilon;
    q_noise = _noise;
}
void agentq_restart(void)
{
    curstate = model_initial_state();

}

double q_maxa(int state, int *pact)
{
    int na = model_num_actions();
    double m = -99999999999.0;
    for (int a=0; a<na; a++) {
        double noise = rand01()*q_noise;
        // adding noise here allow random choice when same values
        double q = qmatrix[q_idx(state, a)]+noise;
        if (q>=m) {
            m = q;
            if (pact) *pact = a;
        }
    }
    return m;
}

int agent_action(int curstate)
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

int q_step(int *retstate)
{
    int action = agent_action(curstate);
    if (action<0) abort();
    if (action>=model_num_actions()) abort();
    
    int newstate = model_new_state(curstate, action);
    double reward = model_reward(newstate);
    
    if (newstate < 0) {
        newstate = model_new_state(curstate, action);
        abort();
    }
    int newidx = q_idx(newstate, action);
    
    double v = q_alpha * (reward + q_gamma*(q_maxa(curstate, NULL)-qmatrix[newidx]));
    qmatrix[newidx] += v;
    curstate = newstate;
    if (retstate) *retstate = newstate;
    if (reward==1) {
        return 1;
    }
    return 0;
}
