//
//  pibt.c
//  train_throttle
//
//  Created by Daniel Braun on 06/03/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//



#include <stdint.h>
#include <memory.h>

#include "pibt.h"

#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/topologyP.h"
#include "../utils/sort_insertion.h"

#ifndef BOARD_HAS_TRKPLN
#error BOARD_HAS_TRKPLN not defined, remove this file from build
#endif

/*
 implementation according to
 article
 Priority inheritance with backtracking for iterative multi-agent path finding
 Keisuke Okumura , Manao Machida , Xavier Défago , Yasumasa Tamura
 
 all comments starting with [1] refer to article,
 and specialy "Algorithm 1 PIBT." page 7
 
 https://pdf.sciencedirectassets.com/271585/1-s2.0-S0004370222X00076/1-s2.0-S0004370222000923/main.pdf?X-Amz-Security-Token=IQoJb3JpZ2luX2VjEJ3//////////wEaCXVzLWVhc3QtMSJGMEQCIHQye5bSDFhVqw/q3Yuf2KqJyqs5a6idRSUMHB+qYC6nAiB/pWGLjvHQlnIcSzVJlRvYegTX8Wt1YdQHDL+I5INn/iq8BQiW//////////8BEAUaDDA1OTAwMzU0Njg2NSIMLvd08hYSgmPm2/nBKpAFViOACMiIPIw+GTo1p7NkGDygpPpfrbA9WXAbyxl7267P0F19CyRDOEHVEV3hC9fnVfvlRENrSh+EWt/AZTVCeSjiJ5fPl4Mta3YKAoKmuHnHe4KLEaNJ7MMcCQq79Al/ycwulk6mi5SgYvqXcQDi1bx619jYtHgj00vGk3SSfXkzdkoHI2X5mk2/1sWcYK51if6pd5aNmgQ3iX9QFJpqAxg/2Sff2gq0Zb3R2sLN812Eoon4uIICHQEoV+RjgSV6MRWG2H7DoKa3Z3cvYnWzYkXvAetwtwh6Mgb7i7arxnz+P/Pv0xDWcihzM8EtqGZ8NhRY12TR6iFw8UcBzolkfP21+ci7VQFCyxD36Ko9i9Z/9KOSSmvsZa/3qQgLN1gyGTIhAHHUmKIlKx9PlBy8gAGvFh9bx+SR9ASDl+bS5DgF4vahLlXuZXK+TP/7Hq+3mflQkRLa9YBSNWOV2LOjubKj+RYwbLb0C9bxzxd/hj7lS83iDN/T0Zn6VcOmok/D7izFrUggFhNw0clnobciP79U/i6KQ9dyPrVoJRW0Ado/W6AmPCKjxZWftDop0U4V6rutn0tFdlbpqJyHeuOcVQUfo6Zil4ZhcVV0uTLyPmZ2jqndLU+DlH/nKaBu2E7+JTpZYLYo8txT1jprZdoGBEUA+VHVqyVk+0rGeHQe9zkDWBQ6/pWm6qDLqcYpH1zbYrnJpqNliPIfYQfcz5zfBsd+d38HugnvgettTo90/TE/iLYy09ofXJ86Yd58Sng2piIAGgUJNTyyX80GJkYnSeASsT/I3R4DC2152hW2HB8M32zT316sKSGjxKGndzl5pVJ2Pqlhbe8Eyx/TIyvvWMNcC9QIJ5vxLLis7Ceb1YEwnI2erwY6sgFJoYiJ1d1IYzAiPSNqoee2iGD9TNUq0T/H85LpICSso3wC/KEvFuzA1Dq7irapwif86APpm0TqDBqNA0AVIedHHqiyNIvzYAnumGxRiZ8vqUvicS3U52HYkbXR/2yNqVvVOW2Np5tiKOmte/Sw/vin6v/fR6AttCSeUmzXb6go+sb8Csdkq3qGP/rU3/KxME/s1ZpnrPDna+p10buxx3vgTD/EOl4LtoWi6YmGpYV3FltN&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20240305T214017Z&X-Amz-SignedHeaders=host&X-Amz-Expires=300&X-Amz-Credential=ASIAQ3PHCVTYQUIQHN43/20240305/us-east-1/s3/aws4_request&X-Amz-Signature=c76809d48ff4f208d15dc239a5fdc4a51f8c0600356d9ab8be9c49f42ce8fccf&hash=fdcaa0e3b20bec56f29cb27b85fc35123f980ec2042df6feb1e8ec43b28e3b4e&host=68042c943591013ac2b2430a89b270f6af2c76d8dfd086a07176afe7c76c2c61&pii=S0004370222000923&tid=spdf-de3b1a76-3c11-4b38-8807-09334efb3112&sid=b17e9f8b5e55234fb98803d4b3c18fe5f3efgxrqb&type=client&tsoh=d3d3LnNjaWVuY2VkaXJlY3QuY29t&ua=1c125c5200045a515455&rr=85fd43768da811b2&cc=fr
 */




#define MAX_AGENT 4
#define MAX_T    20
typedef struct {
    int16_t prio;
    uint8_t target;
    uint8_t pi[MAX_T];
    uint8_t targetdist[256];
} agent_t;

static int nb_agent = 0;

static agent_t agents[MAX_AGENT];

typedef enum {
    valid = 0,
    invalid
} pibt_rc_t;

static void init_target_dist(agent_t *pagent);

static void init_agent(int n, uint8_t startpos, uint8_t targetpos)
{
    // [1] line 1 & 2
    agents[n].prio = n;
    agents[n].target = targetpos;
    agents[n].pi[0] = startpos;
    for (int i = 1; i<MAX_T; i++) {
        agents[n].pi[i] = 0xFF;
    }
    
    init_target_dist(&agents[n]);
}

#pragma mark - PIBT algorithm

#define PRIO_INCREMENT MAX_AGENT

static pibt_rc_t pibt(int t, int agent, int fromagent);

static int pibt_step(int t)
{
    if (t>=MAX_T-1) abort();
    
    // [1] line 3
    // article says "(for each timestep t = 1, 2, . . . until terminates, repeat the following)"
    // but t should start at 0 (right ?)
    int ntodo = 0;
    for (int i = 0; i<nb_agent; i++) {
        if (agents[i].pi[t]==agents[i].target) {
            // i is our epsilon
            agents[i].prio = i;
        } else {
            ntodo++;
            agents[i].prio += PRIO_INCREMENT;
        }
    }
    if (!ntodo) {
        // all done
        return 1;
    }
    // process in decreasing priority
    // [1] line 4,5
    int p = 0xFFFF;
    int nproc = 0;
    for (;;) {
        // get agent with highest prio < p
        int maxp = -1;
        int maxi = -1;
        for (int i=0; i<nb_agent; i++) {
            if (agents[i].prio >= p) continue;
            if (agents[i].prio > maxp) {
                maxp = agents[i].prio;
                maxi = i;
            }
        }
        if (maxi<0) break;
        nproc++;
        if (nproc>nb_agent) {
            // should never occur, if all priorities are unique
            abort();
            break;
        }
        p = maxp;
        // process agent maxi
        // [1] line 6
        if (agents[maxi].pi[t+1] == 0xFF) {
            pibt(t, maxi, -1);
        }
    }
    if (nproc != nb_agent) {
        printf("*** nproc %d\n", nproc);
        //abort();
    }
    return 0;
}

static int cmp_dist_target(lsblk_num_t s1, lsblk_num_t s2, agent_t *pagent)
{
    if (s1.n == -1) return 1;
    if (s2.n == -1) return 0;
    if (pagent->targetdist[s1.n] > pagent->targetdist[s2.n]) return 1;
    return 0;
}
static pibt_rc_t pibt(int t, int agent, int fromagent)
{
    printf("PIBT t=%d agent=%d from=%d\n", t, agent, fromagent);
    // [1] line 9 reachable neighboors plus self
    agent_t *pagent = &agents[agent];
    uint8_t curn = pagent->pi[t];
    if (curn == 0xFF) abort();
    lsblk_num_t C[5];
    C[4].n = curn;
    next_lsblk_nums(C[4], 0, &C[0], &C[1], NULL);
    next_lsblk_nums(C[4], 1, &C[2], &C[3], NULL);
    
    // [1] line 10
    //  sort by increasing order of distance to target
    SORT_INSERTION(lsblk_num_t, C, 5, cmp_dist_target, pagent);
    
    // [1] line 11 : for v ∈ C do
    for (int i=0; i<5; i++) {
        if (C[i].n == -1) break;
        // [1] line 12 : if ∃ ak ∈ A s.t. πk [t + 1] = v then continue
        int exist = 0;
        for (int ak = 0; ak<nb_agent; ak++) {
            agent_t *pak = &agents[ak];
            if (pak->pi[t+1] == C[i].n) {
                // yes it exist
                exist = 1;
                break;
            }
        }
        if (exist) continue;
        // [1] line 13 : if aj ̸=⊥∧πj[t]= v then continue
        if (fromagent != -1) {
            if (agents[fromagent].pi[t] == C[i].n) continue;
        }
        // [1] line 14: πi [t + 1] ← v
        pagent->pi[t+1] = C[i].n;
        
        // [1] line 15: if ∃ ak ∈ A s.t. πk[t] = v ∧ πk[t + 1] = ⊥ then
        exist = 0;
        for (int ak = 0; ak<nb_agent; ak++) {
            agent_t *pak = &agents[ak];
            if ((pak->pi[t] == C[i].n) && (pak->pi[t+1] == 0xFF)) {
                // [1] line 15 : if PIBT(ak , ai ) is invalid then continue
                pibt_rc_t rc = pibt(t, ak, agent);
                if (rc == invalid) {
                    // invalid
                    exist = 1;
                }
                break;
            }
        }
        if (exist) continue;
        
        // [1] line 18: return valid
        return valid;
    }
    // [1] line 20: πi[t+1]←πi[t]
    pagent->pi[t+1] = pagent->pi[t];
    return invalid;
    
    /* 4 dist 2
       5 dist 3
       6 dist 4 */
}


#pragma mark -

static void run_pibt(void)
{
    int t;
    int rc = 0;
    for (t=0; t<MAX_T-1; t++) {
        rc = pibt_step(t);
        if (rc) break;
    }
    printf("\ndone t=%d\n", t);
    for (int a=0; a<nb_agent; a++) {
        printf("T%d (%2d to %2d): ", a, agents[a].pi[0], agents[a].target);
        for (int step=0; step<t+1; step++) {
            printf(" %2d -", agents[a].pi[step]);
        }
        if (rc) printf(" done\n");
        else    printf(" stop\n");
    }
}
void pibt_test1(void)
{
    init_agent(0, 0, 5);
    init_agent(1, 5, 0);
    nb_agent = 2;
    run_pibt();
}

void pibt_test2(void)
{
    init_agent(0, 0, 5);
    init_agent(1, 9, 0);
    init_agent(2, 23, 2);
    nb_agent = 3;
    run_pibt();
}

void pibt_test3(void)
{
    int a=0;
    init_agent(a++, 9, 23);
    init_agent(a++, 0, 9);
    init_agent(a++, 2, 7);
    nb_agent = a;
    run_pibt();
}
void pibt_test(void)
{
    pibt_test1();
    pibt_test2();
    pibt_test3();
    printf("done\n");
}

#pragma mark - initial distance to target

/*
 helper function
 precaculate distance from target
 */

static void update_dist(agent_t *pagent, lsblk_num_t lsblk, int dist)
{
    if (0xFF == pagent->targetdist[lsblk.n]) {
        pagent->targetdist[lsblk.n] = dist;
    } else if (pagent->targetdist[lsblk.n] > dist) {
        pagent->targetdist[lsblk.n] = dist;
    } else {
        return;
    }

    lsblk_num_t a, b, c, d;
    next_lsblk_nums(lsblk, 0, &a, &b, NULL);
    next_lsblk_nums(lsblk, 1, &c, &d, NULL);
    /*if (lsblk.n == 9) {
        printf("bh");
    }*/
    if (a.n != -1) update_dist(pagent, a, dist+1);
    if (b.n != -1) update_dist(pagent, b, dist+1);
    if (c.n != -1) update_dist(pagent, c, dist+1);
    if (d.n != -1) update_dist(pagent, d, dist+1);
}

static void init_target_dist(agent_t *pagent)
{
    for (int i=0; i<255; i++) {
        pagent->targetdist[i] = 0xFF;
    }
    lsblk_num_t lsblk;
    lsblk.n = pagent->target;
    update_dist(pagent, lsblk, 0);
}
