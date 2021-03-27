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

/*
 
    0 -----\
      ______\_____ _______
        1      2     3
 */
static track_segment_t trseg[] = {
    /*0*/ {0xFF, 0xFF, 2,    0xFF},
    /*1*/ {0xFF, 0xFF, 2,    0xFF},
    /*2*/ {1,    0,    3,    0xFF},
    /*3*/ {2,    0,    0xFF, 0xFF}
};

static void update_score(trstate_t *st);


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

int update_state(trstate_t *st, track_segment_t *trseg, uint16_t step)
{
    int rc = 0;
    for (int i=0; i<MAX_TRAINS; i++) {
        uint8_t s = st->t[i];
        if (0xFF == s) continue;
        uint16_t b = trbits_gettrain(step, i);
        int m = trbits_motion(b);
        int d = trbits_nextdir(b);
        if (!m) continue;
        uint8_t ns1;
        uint8_t ns2;
        switch (m) {
            case -1:
                ns1 = trseg[s].left_1;
                ns2 = trseg[s].left_2;
                break;
            case 1:
                ns1 = trseg[s].right_1;
                ns2 = trseg[s].right_2;
                break;
            default:
                abort();
                break;
        }
        int ns = ns1;
        if ((ns2 != 0xFF) & d) {
            ns = ns2;
        }
        if (ns == 0xFF) {
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
        if (ns != 0xFF) st->t[i] = ns;
    }
    update_score(st);
    return rc;
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

int update_state_all(trstate_t *st, track_segment_t *trseg, tplan_t *p)
{
    int rc = 0;
    st->score = 0;
    for (int i=0; i<MAX_T; i++) {
        rc |= update_state(st, trseg, p->step[i]);
        if ((0)) print_state(st);
    }
    if ((0)) printf("score : %d\n", st->score);
    p->score = st->score;
    return rc;
}

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


static void update_score(trstate_t *st)
{
    if (st->flags & RC_COL) {
        st->score -= 30;
    }
    if (st->flags & RC_OUT) {
        st->score -= 20;
    }
    // hardcoded
    if (st->t[0]==3) {
        st->score += 20;
    } else {
        st->score -= 2;
    }
    if (st->t[1]==1) {
        st->score += 20;
    } else{
        st->score -= 2;
    }
}

#pragma mark -

#define NUM_POPULATION 16

static tplan_t population[NUM_POPULATION];

static void set_random_trkseg(tplan_t *tseg)
{
    memset(tseg, 0, sizeof(*tseg));
    for (int i=0; i<MAX_T; i++) {
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
    int n = (rand()%(MAX_T-2))+1;
    for (int i=0; i<MAX_T; i++) {
        res->step[i] = (i<n) ? p1->step[i] : p2->step[i];
    }
}

void test_me(void)
{
    init_testplan();
    trstate_t state;
    intial_state(&state);
    print_state(&state);
    int rc = update_state_all(&state, trseg, &testplan);
    printf("hop\n");
    for (int i=0; i<NUM_POPULATION; i++) {
        set_random_trkseg(&population[i]);
    }
    srand((unsigned int)time(NULL));
    for (int gen = 0; gen<100; gen ++) {
        printf("\n---------------- g%d\n", gen);
        double m = 0;
        for (int pop = 0; pop<NUM_POPULATION; pop++) {
            intial_state(&state);
            update_state_all(&state, trseg, &population[pop]);
            printf("individu %d score %d\n", pop, population[pop].score);
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
        for (int p = 0; p<NUM_POPULATION; p++) {
            if (population[p].mark) continue;
            int p1,p2;
            p1 = get_parent(population, NUM_POPULATION);
            do {
                p2 = get_parent(population, NUM_POPULATION);
            } while (p2 != p1);
            crossover(&population[p], &population[p1], &population[p2]);
        }
        // mutation
        for (int i=0; i<20; i++) {
            int p = rand()%NUM_POPULATION;
            int t = rand()%MAX_T;
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
    printf("done\n");
    exit(0);
}
