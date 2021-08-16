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

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

//#define SRAND_VAL       2
/*
 per train:
 X X            motion   01=<-  11=-> 00=idle  (10=unused)
      X         next-dir
 16bit contains up to 5 trains, MSB not used
 */
static inline uint16_t trbits_gettrain(uint16_t bits, int8_t ntrain)
{
    uint16_t v = (bits >> (3*ntrain)) & 0x7;
    return v;
}
static inline int16_t trbits_motion(int16_t bits)
{
    int16_t t = (bits & 0x6)>>1;
    switch (t) {
        case 0: return 0;
        case 1: return 1;
        case 3: return -1;
        default: abort();
            break;
    }
    return 0;
}
static inline uint16_t trbits_nextdir(int16_t bits)
{
    return bits & 0x01;
}

static inline void set_motion_dir(uint16_t *pbits, int trn, int motion, int nextdir)
{
    uint16_t t = ((uint16_t)(motion & 0x3))<<1 | (nextdir & 1);
    uint16_t m = 0x7;
    t = t<<(trn*3);
    m = m<<(trn*3);
    *pbits &= ~m;
    *pbits |= t;
    
    if ((1)) {
        int m = trbits_motion(trbits_gettrain(*pbits, trn));
        int d = trbits_nextdir(trbits_gettrain(*pbits, trn));
        if (m != motion) abort();
        if (d != nextdir) abort();
    }
}


/*
 * tplan_t is individual in population
 * it contains step (direction for each trains among time)
 */
typedef  struct {
    uint16_t step[MAX_TIMESTEP];
    int16_t score;
    uint8_t mark;
} tplan_t;


// ------------------------------------------------------------------

#if HARDCODED_TOPOLOGY
/* ---------------
 * track_segment_t
 * holds topology
 */
typedef struct {
    uint8_t left_1;
    uint8_t left_2;
    uint8_t right_1;
    uint8_t right_2;
} track_segment_t;

#endif

// ------------------------------------------------------------------

typedef struct {
    int8_t s[MAX_SEGM];
    // more ?
} segstate_t;

typedef struct {
    uint8_t t[MAX_TRAINS];
    uint8_t flags;
    int16_t score;
} trstate_t;

typedef struct{
    uint8_t t[MAX_TRAINS];
} trtarget_t;

#define RC_OK  0
#define RC_OUT 1
#define RC_COL 2

// -------------------------------------------------------------------------


//static int get_segstate(trstate_t *st, segstate_t *retseg);
static int update_state(trstate_t *st, uint16_t step, trtarget_t *t, int score);
static int update_state_all(trstate_t *st, tplan_t *p, trtarget_t *t);
static void update_score(trstate_t *st, trtarget_t *tg, int step);

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

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------


static void intial_state(trstate_t   *st)
{
    memset(st, 0, sizeof(*st));
   
    for (int i=2; i<MAX_TRAINS; i++) {
        st->t[i]=0xFF;
    }
}



static int update_state(trstate_t *st, uint16_t step, trtarget_t *target, int stepnum)
{
    int rc = 0;
    for (int i=0; i<MAX_TRAINS; i++) {
        uint8_t s = st->t[i];
        if (0xFF == s) continue;
        uint16_t b = trbits_gettrain(step, i);
        int m = trbits_motion(b);
        int d = trbits_nextdir(b);
        if (!m) {
            st->score += 1; // TODO this should be in update_score
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
        if (ns == -1) ns = ns2;
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
    update_score(st, target, stepnum);
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
            int ns;
            if (ns1==0xFF) ns1 = -1;
            if (ns2==0xFF) ns2 = -1;
            if (ns1 == -1) {
                ns = ns2;
            } else if (ns2 == -1) {
                ns = ns1;
            } else {
                ns = d ? ns2 : ns1;
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

static int update_state_all(trstate_t *st, tplan_t *p, trtarget_t *target)
{
    int rc = 0;
    st->score = 0;
    for (int i=0; i<MAX_TIMESTEP; i++) {
        rc |= update_state(st, p->step[i], target, i);
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

#define SCORE_PRINT (0)
static void update_score(trstate_t *st, trtarget_t *tg, int stepnum)
{
    if ((SCORE_PRINT)) printf("score upd st t0=%d t1=%d\n", st->t[0], st->t[1]);
    if (st->flags & RC_COL) {
        if ((SCORE_PRINT)) printf("  score COL -30\n");
        st->score -= 50;
    }
    if (st->flags & RC_OUT) {
        if ((SCORE_PRINT)) printf("  score OUT -20\n");
        st->score -= 30;
    }
    // hardcoded
    for (int i=0; i<MAX_TRAINS; i++) {
        if (tg->t[i] == 0xFF) continue;
        if (st->t[i] == tg->t[i]) {
            if ((SCORE_PRINT)) printf("  score match +20 : st->t[%d] = target %d\n", i, tg->t[i]);
            st->score += (stepnum == MAX_TIMESTEP-1) ? 60 : 30;
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

static trstate_t  initialstate;
static trtarget_t target;
static uint8_t target_changed = 0;
static int bestIdx = 0;

void trkpln_set_train_pos(int trnum, int segnum)
{
    initialstate.t[trnum] = segnum;
}
void trkpln_set_train_target(int trnum, int segnum)
{
    target.t[trnum] = segnum;
    target_changed = 1;
}

void trkpln_init(void)
{
    intial_state(&initialstate);
    for (int i=0; i<MAX_TRAINS; i++) target.t[i] = 0xFF;
#ifdef SRAND_VAL
    srand(SRAND_VAL);
#else
    srand((unsigned int)time(NULL));
    #endif
}
#pragma mark -

void trkpln_process(void)
{
    if (!target_changed) return;
    target_changed = 0;
    
    trstate_t state;

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
    bestIdx = 0;
    for (int i=0; i<NUM_POPULATION; i++) {
        if (population[i].score > bestScore) {
            bestScore = population[i].score;
            bestIdx = i;
        }
    }
    printf("best score i=%d score=%d\n", bestIdx, bestScore);
    return;
}

void trkpln_get_route(int trnum)
{
    print_route(&population[bestIdx], &initialstate);
}



#pragma mark -
void test_me(void)
{
    trkpln_init();
    trkpln_set_train_pos(0, 0);
    trkpln_set_train_pos(1, 2);
    
    trkpln_set_train_target(0, 2);
    trkpln_set_train_target(1, 0);

    trkpln_process();
    
    trkpln_get_route(0);
    printf("done\n");
     exit(0);


}
