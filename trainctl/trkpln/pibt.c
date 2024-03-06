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


#ifndef BOARD_HAS_TRKPLN
#error BOARD_HAS_TRKPLN not defined, remove this file from build
#endif

#define MAX_AGENT 4
#define MAX_T    10
typedef struct {
    int16_t prio;
    uint8_t target;
    uint8_t pi[MAX_T];
} agent_t;

static int nb_agent = 0;

static agent_t agents[MAX_AGENT];

/*
 implementation according to
 article
 Priority inheritance with backtracking for iterative multi-agent path finding
 Keisuke Okumura , Manao Machida , Xavier Défago , Yasumasa Tamura
 
 all comments starting with [1] refer to article,
 and specialy "Algorithm 1 PIBT." page 7
 
 https://pdf.sciencedirectassets.com/271585/1-s2.0-S0004370222X00076/1-s2.0-S0004370222000923/main.pdf?X-Amz-Security-Token=IQoJb3JpZ2luX2VjEJ3//////////wEaCXVzLWVhc3QtMSJGMEQCIHQye5bSDFhVqw/q3Yuf2KqJyqs5a6idRSUMHB+qYC6nAiB/pWGLjvHQlnIcSzVJlRvYegTX8Wt1YdQHDL+I5INn/iq8BQiW//////////8BEAUaDDA1OTAwMzU0Njg2NSIMLvd08hYSgmPm2/nBKpAFViOACMiIPIw+GTo1p7NkGDygpPpfrbA9WXAbyxl7267P0F19CyRDOEHVEV3hC9fnVfvlRENrSh+EWt/AZTVCeSjiJ5fPl4Mta3YKAoKmuHnHe4KLEaNJ7MMcCQq79Al/ycwulk6mi5SgYvqXcQDi1bx619jYtHgj00vGk3SSfXkzdkoHI2X5mk2/1sWcYK51if6pd5aNmgQ3iX9QFJpqAxg/2Sff2gq0Zb3R2sLN812Eoon4uIICHQEoV+RjgSV6MRWG2H7DoKa3Z3cvYnWzYkXvAetwtwh6Mgb7i7arxnz+P/Pv0xDWcihzM8EtqGZ8NhRY12TR6iFw8UcBzolkfP21+ci7VQFCyxD36Ko9i9Z/9KOSSmvsZa/3qQgLN1gyGTIhAHHUmKIlKx9PlBy8gAGvFh9bx+SR9ASDl+bS5DgF4vahLlXuZXK+TP/7Hq+3mflQkRLa9YBSNWOV2LOjubKj+RYwbLb0C9bxzxd/hj7lS83iDN/T0Zn6VcOmok/D7izFrUggFhNw0clnobciP79U/i6KQ9dyPrVoJRW0Ado/W6AmPCKjxZWftDop0U4V6rutn0tFdlbpqJyHeuOcVQUfo6Zil4ZhcVV0uTLyPmZ2jqndLU+DlH/nKaBu2E7+JTpZYLYo8txT1jprZdoGBEUA+VHVqyVk+0rGeHQe9zkDWBQ6/pWm6qDLqcYpH1zbYrnJpqNliPIfYQfcz5zfBsd+d38HugnvgettTo90/TE/iLYy09ofXJ86Yd58Sng2piIAGgUJNTyyX80GJkYnSeASsT/I3R4DC2152hW2HB8M32zT316sKSGjxKGndzl5pVJ2Pqlhbe8Eyx/TIyvvWMNcC9QIJ5vxLLis7Ceb1YEwnI2erwY6sgFJoYiJ1d1IYzAiPSNqoee2iGD9TNUq0T/H85LpICSso3wC/KEvFuzA1Dq7irapwif86APpm0TqDBqNA0AVIedHHqiyNIvzYAnumGxRiZ8vqUvicS3U52HYkbXR/2yNqVvVOW2Np5tiKOmte/Sw/vin6v/fR6AttCSeUmzXb6go+sb8Csdkq3qGP/rU3/KxME/s1ZpnrPDna+p10buxx3vgTD/EOl4LtoWi6YmGpYV3FltN&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20240305T214017Z&X-Amz-SignedHeaders=host&X-Amz-Expires=300&X-Amz-Credential=ASIAQ3PHCVTYQUIQHN43/20240305/us-east-1/s3/aws4_request&X-Amz-Signature=c76809d48ff4f208d15dc239a5fdc4a51f8c0600356d9ab8be9c49f42ce8fccf&hash=fdcaa0e3b20bec56f29cb27b85fc35123f980ec2042df6feb1e8ec43b28e3b4e&host=68042c943591013ac2b2430a89b270f6af2c76d8dfd086a07176afe7c76c2c61&pii=S0004370222000923&tid=spdf-de3b1a76-3c11-4b38-8807-09334efb3112&sid=b17e9f8b5e55234fb98803d4b3c18fe5f3efgxrqb&type=client&tsoh=d3d3LnNjaWVuY2VkaXJlY3QuY29t&ua=1c125c5200045a515455&rr=85fd43768da811b2&cc=fr
 */

static void init_agent(int n, uint8_t startpos, uint8_t targetpos)
{
    // [1] line 1 & 2
    agents[n].prio = n;
    agents[n].target = targetpos;
    agents[n].pi[0] = startpos;
    for (int i = 1; i<MAX_T; i++) {
        agents[n].pi[i] = 0xFF;
    }
}
#define PRIO_INCREMENT MAX_AGENT

static int pibt(int t, int agent, int fromagent);

static void pibt_step(int t)
{
    if (t>=MAX_T-1) abort();
    
    // [1] line 3
    // article says "(for each timestep t = 1, 2, . . . until terminates, repeat the following)"
    // but t should start at 0 (right ?)
    for (int i = 0; i<nb_agent; i++) {
        if (agents[i].pi[t]==agents[i].target) {
            // i is our epsilon
            agents[i].prio = i;
        } else {
            agents[i].prio += PRIO_INCREMENT;
        }
    }
    // process in decreasing priority
    // [1] line 4,5
    int p = 0xFFFF;
    int nproc = 0;
    for (;;) {
        // get agent with highest prio < p
        int maxp = 0;
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
            break;
        }
        p = maxp;
        // process agent maxi
        // [1] line 6
        if (agents[maxi].pi[t+1] == 0xFF) {
            pibt(t, maxi, -1);
        }
    }
    if (nproc != nb_agent) abort();
}

static int pibt(int t, int agent, int maxagent)
{
    printf("PIBT t=%d agent=%d from=%d\n", t, agent, maxagent);
    return 0;
}
#pragma mark -


void pibt_test1(void)
{
    init_agent(0, 0, 5);
    init_agent(1, 5, 0);
    nb_agent = 2;
    for (int t=0; t<MAX_T-1; t++) {
        pibt_step(t);
    }
}


