//
//  agentQ.h
//  trackplan2
//
//  Created by Daniel BRAUN on 05/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef agentQ_h
#define agentQ_h

#include <stdio.h>

void agentq_init(double alpha, double gamma, double epsilon);
void agentq_restart(void);


int q_step(int *retstate);

#endif /* agentQ_h */
