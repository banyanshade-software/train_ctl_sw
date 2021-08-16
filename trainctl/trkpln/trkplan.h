//
//  trkplan.h
//  trackplan
//
//  Created by Daniel BRAUN on 10/03/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef trkplan_h
#define trkplan_h

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_TIMESTEP        12
#define MAX_TRAINS          4
#define MAX_SEGM            32  // only a safeguard
#define HARDCODED_TOPOLOGY  0

#define NUM_POPULATION      32
#define NUM_GENERATIONS     3000
#define PERCENT_CROSS5      60
#define NUM_MUTATIONS       50


void trkpln_init(void);

void trkpln_set_train_pos(int trnum, int segnum);
void trkpln_set_train_target(int trnum, int segnum);

void trkpln_process(void);
void trkpln_get_route(int trnum);



void test_me(void);

#endif /* trkplan_h */
