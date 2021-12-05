//
//  model.c
//  trackplan2
//
//  Created by Daniel BRAUN on 05/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdlib.h>
#include <math.h>

#include "model.h"
#include "topology.h"
#include "topologyP.h"

#define LIMIT_TWO_TRAINS 0


static int numtrain=0;
static int numblk=0;

static int from[4] = {0};
static int dest[4] = {0};

static int finalstate = -1;

void model_setup(int numtrains)
{
    numtrain = numtrains;
    numblk = topology_num_sblkd();
    if (numtrain>4) numtrain = 4;
#if LIMIT_TWO_TRAINS
    if (numtrain>2) numtrain = 2;
#endif
    finalstate = -1;
    memset(&from, 0, sizeof(from));
    memset(&dest, 0, sizeof(dest));
}
void model_set_from_to(int train, int fr, int to)
{
    from[train] = fr;
    dest[train] = to;
    finalstate = model_state_num(dest[0], dest[1], dest[2], dest[3]);
}


int model_num_states(void)
{
    int n = 1;
    for (int i=0; i<numtrain; i++) {
        n *= numblk;
    }
    return n;
}



// state num <-> info
int model_state_num(int posT0, int posT1, int posT2, int posT3)
{
    int p = posT0;
    if (posT1>=0) p += posT1 * numblk;
    if (posT2>=0) p += posT2 * numblk * numblk;
    if (posT3>=0) p += posT3  * numblk * numblk *numblk;
    if (p >= model_num_states()) {
        abort();
    }
    return p;
}
void model_positions_for_state(int statenum, int *posT0, int *posT1, int *posT2, int *posT3)
{
    *posT0 = statenum % numblk;
    statenum /= numblk;
    *posT1 = statenum % numblk;
    statenum /= numblk;
    *posT2 = statenum % numblk;
    statenum /= numblk;
    *posT3 = statenum % numblk;
    statenum /= numblk;
}

int model_initial_state(void)
{
    for (int i=numtrain; i<4; i++) {
        from[i] = 0;
    }
    return model_state_num(from[0], from[1], from[2], from[3]);
}
int model_final_state(void)
{
    return finalstate;
}

int model_num_actions(void)
{
    return NUM_ELEM_ACTIONS*numtrain;
}

// action_num <-> actions
int model_action_num(model_elem_action_t T0act, model_elem_action_t T1act, model_elem_action_t T2act, model_elem_action_t T3act)
{
    int a;
    int na = model_num_actions();
    a =  T0act;
    a += T1act * na;
    a += T2act * na * na;
    a += T3act * na * na;
    return a;
}
void model_actions_for_num(int actionnum, model_elem_action_t *T0act, model_elem_action_t *T1act, model_elem_action_t *T2act, model_elem_action_t *T3act)
{
    //int na = model_num_actions();
    *T0act = actionnum % NUM_ELEM_ACTIONS;
    actionnum /= NUM_ELEM_ACTIONS;
    *T1act = actionnum % NUM_ELEM_ACTIONS;
    actionnum /= NUM_ELEM_ACTIONS;
    *T2act = actionnum % NUM_ELEM_ACTIONS;
    actionnum /= NUM_ELEM_ACTIONS;
    *T3act = actionnum % NUM_ELEM_ACTIONS;
    actionnum /= NUM_ELEM_ACTIONS;
}

// transitions
int model_new_state(int statenum, int actionnum)
{
    model_elem_action_t a[4];
    int p[4];
    model_actions_for_num(actionnum, &a[0], &a[1], &a[2], &a[3]);
    model_positions_for_state(statenum, &p[0], &p[1], &p[2], &p[3]);
    for (int i = 0; i<3; i++) {
        if (i>=numtrain) break;
        const topo_lsblk_t *co = topology_get_sblkd(p[i]);
        switch (a[i]) {
            case action_dont_move:
                break;
            case action_left_straight:
                if (co->left1 == -1) break;
                p[i] = co->left1;
                break;
            case action_left_turn:
                if (co->left2 == -1) break;
                p[i] = co->left2;
                break;
            case action_right_straight:
                if (co->right1 == -1) break;
                p[i] = co->right1;
                break;
            case action_right_turn:
                if (co->right2 == -1) break;
                p[i] = co->right2;
                break;
            default:
                abort();
                break;
        }
    }
    int ns = model_state_num(p[0], p[1], p[2], p[3]);
    return ns;
}

// reward
double model_reward(int statenum)
{
    if (statenum == finalstate) return 1.0;
    int p[4];
    model_positions_for_state(statenum, &p[0], &p[1], &p[2], &p[3]);

    int r = 0;
    for (int i=0; i<numtrain; i++) {
        for (int j=0; j<numtrain; j++) {
            if (j==i) continue;
            if (p[i]==p[j]) r -= 0.5;
        }
    }
    return r;
}
