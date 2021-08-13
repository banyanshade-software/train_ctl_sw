//
//  trkplan.c
//  trackplan
//
//  Created by Daniel BRAUN on 10/03/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include "trkplan.h"
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#include "topology.h"


#if HARDCODED_TOPOLOGY
/* hardcoded : (HARDCODED_TOPOLOGY=1)
 
    0 -----\
      ______\_____ _______
        1      2     3
 */
static track_segment_t gTrseg[] = {
    /*0*/ {0xFF, 0xFF, 2,    0xFF},
    /*1*/ {0xFF, 0xFF, 2,    0xFF},
    /*2*/ {1,    0,    3,    0xFF},
    /*3*/ {2,    0xFF,    0xFF, 0xFF}
};
#endif

static void update_score(trstate_t *st, trtarget_t *tg);


static void intial_state(trstate_t
                         *st)
{
    memset(st, 0, sizeof(*st));
    st->t[0]=1;
    st->t[1]=3;
    for (int i=2; i<MAX_TRAINS; i++) {
        st->t[i]=0xFF;
    }
}
int get_segstate(trstate_t *st, segstate_t *retseg)
{
    memset(retseg, 0, sizeof(*retseg));
    for (int i=0; i<MAX_TRAINS; i++) {
        uint8_t s = st->t[i];
        if (0xFF == s) continue;
        if (s>=MAX_SEGM) abort();
        retseg->s[s]=i;
    }
    return 0;
}

int update_state(trstate_t *st, uint16_t step, trtarget_t *target)
{
    int rc = 0;
    for (int i=0; i<MAX_TRAINS; i++) {
        uint8_t s = st->t[i];
        if (0xFF == s) continue;
        uint16_t b = trbits_gettrain(step, i);
        int m = trbits_motion(b);
        int d = trbits_nextdir(b);
        if (!m) continue;
        int ns1;
        int ns2;
#if HARDCODED_TOPOLOGY
        switch (m) {
            case -1:
                ns1 = gTrseg[s].left_1;
                ns2 = gTrseg[s].left_2;
                break;
            case 1:
                ns1 = gTrseg[s].right_1;
                ns2 = gTrseg[s].right_2;
                break;
            default:
                abort();
                break;
        }
#else
        int tn;
        if (m < 0) {
            next_blocks_nums(s, 1, &ns1, &ns2, &tn);
        } else if (m > 0) {
            next_blocks_nums(s, 0, &ns1, &ns2, &tn);
        } else {
           abort();
        }
#endif
        if (ns1==0xFF) ns1 = -1;
        if (ns2==0xFF) ns2 = -1;
        int ns = ns1;
        if ((ns2 != -1) & d) {
            ns = ns2;
        }
        if (ns == -1) {
            rc |= RC_OUT;
            st->flags |= RC_OUT;
        } else {
            for (int j=0; j<MAX_TRAINS; j++) {
                if (j==i) continue;
                if (st->t[j] == ns) {
                    rc |= RC_COL;
                    st->flags |= RC_COL;
                    break;
                }
            }
        }
        if (ns != -1) st->t[i] = ns;
    }
    update_score(st, target);
    return rc;
}


static void print_route(tplan_t *p, trstate_t *initialstate)
{
    for (int i=0; i<MAX_TRAINS; i++) {
        trstate_t state = *initialstate;
        printf("route T%d : %d ", i, state.t[i]);
        for (int stp=0; stp < MAX_TIMESTEP; stp++) {
            //uint16_t step[MAX_TIMESTEP];
            uint16_t step = p->step[stp];
            uint8_t s = state.t[i];
            if (0xFF == s) continue;
            uint16_t b = trbits_gettrain(step, i);
            int m = trbits_motion(b);
            int d = trbits_nextdir(b);
            if (!m) {
                printf(" -");
                continue;
            }
            int ns1;
            int ns2;
#if HARDCODED_TOPOLOGY
            switch (m) {
            case -1:
                ns1 = gTrseg[s].left_1;
                ns2 = gTrseg[s].left_2;
                break;
            case 1:
                ns1 = gTrseg[s].right_1;
                ns2 = gTrseg[s].right_2;
                break;
            default:
                abort();
                break;
            }
#else
            int tn;
            if (m < 0) {
                next_blocks_nums(s, 1, &ns1, &ns2, &tn);
            } else if (m > 0) {
                next_blocks_nums(s, 0, &ns1, &ns2, &tn);
            } else {
                abort();
            }
#endif
            if (ns1==0xFF) ns1 = -1;
            if (ns2==0xFF) ns2 = -1;
            int ns = ns1;
            if ((ns2 != -1) & d) {
                ns = ns2;
            }
            printf(" %d", ns);
            if (ns == -1) {
                break;
            }
            state.t[i] = ns;
        } // stp
        printf("\n");
    } // trains

}

static void print_state(trstate_t *st)
{
    for (int i = 0; i<MAX_TRAINS; i++) {
        uint8_t s = st->t[i];
        if (s==0xFF) continue;
        printf("T%d seg %d   ", i, s);
    }
    printf("  %s %s\n", (st->flags & RC_COL) ? "COL" :"", (st->flags & RC_OUT) ? "OUT":"");
}

int update_state_all(trstate_t *st, tplan_t *p, trtarget_t *target)
{
    int rc = 0;
    st->score = 0;
    for (int i=0; i<MAX_TIMESTEP; i++) {
        rc |= update_state(st, p->step[i], target);
        if ((0)) print_state(st);
    }
    if ((0)) printf("score : %d\n", st->score);
    p->score = st->score;
    return rc;
}

#if 0
static tplan_t testplan;

static void init_testplan(void)
{
    int i=0;
    memset(&testplan, 0, sizeof(tplan_t));
    set_motion_dir(&testplan.step[i], 0,  0, 0);       set_motion_dir(&testplan.step[i++], 1,  -1, 0);
    set_motion_dir(&testplan.step[i], 0,  0, 0);       set_motion_dir(&testplan.step[i++], 1,  -1, 1);
    set_motion_dir(&testplan.step[i], 0,  1, 0);       set_motion_dir(&testplan.step[i++], 1,  0, 0);
    set_motion_dir(&testplan.step[i], 0,  1, 0);       set_motion_dir(&testplan.step[i++], 1,  0, 0);
    set_motion_dir(&testplan.step[i], 0,  0, 0);       set_motion_dir(&testplan.step[i++], 1,  1, 0);
    set_motion_dir(&testplan.step[i], 0,  0, 0);       set_motion_dir(&testplan.step[i++], 1,  -1, 0);
}
#endif


static void update_score(trstate_t *st, trtarget_t *tg)
{
    if (st->flags & RC_COL) {
        st->score -= 30;
    }
    if (st->flags & RC_OUT) {
        st->score -= 20;
    }
    // hardcoded
    for (int i=0; i<MAX_TRAINS; i++) {
        if (st->t[i] == tg->t[i]) {
            st->score += 20;
        } else {
            st->score -= 2;
        }
    }
}

#pragma mark -


static tplan_t population[NUM_POPULATION];

static void set_random_trkseg(tplan_t *tseg)
{
    memset(tseg, 0, sizeof(*tseg));
    for (int i=0; i<MAX_TIMESTEP; i++) {
        for (int t=0; t<MAX_TRAINS; t++) {
            set_motion_dir(&tseg->step[i], t,  rand()%3-1, rand()%2);
        }
    }
}

#pragma mark -

static int get_parent(tplan_t *p, int n)
{
    for (;;) {
        int i = rand() % n;
        if (p[i].mark) return i;
    }
    // not reached
    return 0;
}
static void crossover(tplan_t *res, tplan_t *p1, tplan_t *p2)
{
    int n = (rand()%(MAX_TIMESTEP-2))+1;
    for (int i=0; i<MAX_TIMESTEP; i++) {
        res->step[i] = (i<n) ? p1->step[i] : p2->step[i];
    }
}


#pragma mark -

void test_me(void)
{
    trstate_t state;
    trstate_t initialstate;
    trtarget_t target;
    for (int i=0; i<MAX_TRAINS; i++) target.t[i] = 0xFF;
    target.t[0] = 3;
    target.t[1] = 1;
    /*
    init_testplan();
    intial_state(&state);
    print_state(&state);
    int rc = update_state_all(&state, trseg, &testplan);
    printf("hop\n");
     */
#ifdef SRAND_VAL
    srand(SRAND_VAL);
#else
    srand((unsigned int)time(NULL));
#endif
    
    intial_state(&initialstate);
    for (int i=0; i<NUM_POPULATION; i++) {
        set_random_trkseg(&population[i]);
    }
    for (int gen = 0; gen<NUM_GENERATIONS; gen ++) {
        printf("\n---------------- g%d\n", gen);
        double m = 0;
        for (int pop = 0; pop<NUM_POPULATION; pop++) {
            memcpy(&state, &initialstate, sizeof(state));
            update_state_all(&state, &population[pop], &target);
            printf("individu %d score %d\n", pop, population[pop].score);
            if ((0)) print_route(&population[pop], &initialstate);
            m += population[pop].score;
        }
        m /= NUM_POPULATION;
        printf("average score : %f\n", m);
        int nkill = 0;
        for (int pop = 0; pop<NUM_POPULATION; pop++) {
            if (population[pop].score<m) {
                population[pop].mark = 0;
                nkill++;
            } else {
                population[pop].mark = 1;
            }
        }
        int nk2 = 0;
        // bof bof il faut prendre la médiane
        printf("nkill = %d\n", nkill);
        while (nkill < NUM_POPULATION/2) {
            for (int pop = 0; pop<NUM_POPULATION; pop++) {
                if ((population[pop].score<=m) && (population[pop].mark)) {
                    population[pop].mark = 0;
                    nkill++; nk2++;
                    if (nkill>=NUM_POPULATION/2) break;
                }
            }
            m = m+10;
        }
        printf("nk2=%d\n", nk2);
        // crossover
        for (int nc = 0; nc<5; nc++) {
            for (int p = 0; p<NUM_POPULATION; p++) {
                if (population[p].mark) continue;
                if ((rand()%100) < PERCENT_CROSS5) continue;
                int p1,p2;
                p1 = get_parent(population, NUM_POPULATION);
                do {
                    p2 = get_parent(population, NUM_POPULATION);
                } while (p2 != p1);
                crossover(&population[p], &population[p1], &population[p2]);
            }
        }
        // mutation
        for (int i=0; i<NUM_MUTATIONS; i++) {
            int p = rand()%NUM_POPULATION;
            int t = rand()%MAX_TIMESTEP;
            int tr = rand()%MAX_TRAINS;
            int motion =  trbits_motion(trbits_gettrain(population[p].step[t], tr));
            int nextdir = trbits_nextdir(trbits_gettrain(population[p].step[t], tr));
            if (rand()%2) {
                nextdir = rand()%2;
            } else {
                motion = rand()%3-1;
            }
            set_motion_dir(&population[p].step[t], tr, motion, nextdir);
        }
    }
    int bestScore = 0;
    int bestIdx = 0;
    for (int i=0; i<NUM_POPULATION; i++) {
        if (population[i].score > bestScore) {
            bestScore = population[i].score;
            bestIdx = i;
        }
    }
    printf("best score i=%d score=%d\n", bestIdx, bestScore);
    print_route(&population[bestIdx], &initialstate);
    printf("done\n");
    exit(0);
}
