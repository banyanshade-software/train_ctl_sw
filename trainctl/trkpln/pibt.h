//
//  pibt.h
//  train_throttle
//
//  Created by Daniel Braun on 06/03/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#ifndef pibt_h
#define pibt_h


// https://kei18.github.io/pibt2/
/*
 the problem with PIBT is that it will backtrack only on a single time episode
 path finding may arrive in a position where all trains are stuck on after the other
 on a deadend track.
 
 PIBT ensure reachability only on biconnected graph
 (this is the case for warehouse, not for us)
 model railway tracks includes many deadend tracks, usable to avoid collision, but
 making the graph not biconnected
 
 
 winPIBT
 https://kei18.github.io/pibt/
 https://arxiv.org/abs/1905.10149
 
 */


#include "../misc.h"
#include "../msg/tasklet.h"

void pibt_test(void);
void pibt_test1(void);
void pibt_test2(void);
void pibt_test3(void);

#endif /* pibt_h */
