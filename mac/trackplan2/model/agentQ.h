//
//  agentQ.h
//  trackplan2
//
//  Created by Daniel BRAUN on 05/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef agentQ_h
#define agentQ_h

//#include <stdio.h>
#include "agent.h"



extern agent_def_t q_agent;

/*
void agentq_init(void);
void agentq_setparams(double _alpha, double _gamma, double _epsilon, double _noise);
void q_param(double,double,double,double,double,double,double,double);
void agentq_restart(void);


int q_step(int *retstate);
*/
void q_dump_state(int st);

#endif /* agentQ_h */
