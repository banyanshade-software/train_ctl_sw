//
//  agent.h
//  trackplan2
//
//  Created by Daniel BRAUN on 06/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef agent_h
#define agent_h

typedef struct {
    void (*init)(void);
    void (*restart)(void);
    int  (*step)(int *retstate);
    void (*setparam)(double,double,double,double,double,double,double,double);
} agent_def_t;


#endif /* agent_h */
