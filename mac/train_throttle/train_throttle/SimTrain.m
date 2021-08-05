//
//  SimTrain.m
//  train_throttle
//
//  Created by Daniel BRAUN on 10/10/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "SimTrain.h"
#import "railconfig.h"
#include "topology.h"

@interface SimTrain ()
//@property (nonatomic,readwrite) double speed;
@end


@implementation SimTrain {
    double volt[NUM_CANTONS];
    double pwm[NUM_CANTONS];
    int dir[NUM_CANTONS];
    double bemf[NUM_CANTONS];
    
    double speed[NUM_TRAINS];
    double position[NUM_TRAINS];

    int c1[NUM_TRAINS];
    int cold[NUM_TRAINS];
}

@synthesize speed = _speed;

- (instancetype) init
{
    self = [super init];
    if (!self) return nil;
    for (int i=0; i<NUM_TRAINS; i++) {
        c1[i] = -1;
        cold[i] = -1;
        speed[i] = 0;
        position[i] = 0;
    }
    for (int i=0; i<NUM_CANTONS; i++) {
        bemf[i] = 0.0;
        volt[i] = 2.0;
        pwm[i] = 0;
        dir[i] = 0;
    }
    c1[0] = 1;
   
    return self;
}

- (void)setVolt:(double)v forCantonNum:(int)numc
{
    NSAssert(numc>=0, @"bad numc");
    NSAssert(numc<NUM_CANTONS, @"bad numc");
    volt[numc] = v;
}
- (void)setPwm:(double)p dir:(int)d forCantonNum:(int)numc
{
    NSAssert(numc>=0, @"bad numc");
    NSAssert(numc<NUM_CANTONS, @"bad numc");
    pwm[numc] = p;
    dir[numc] = d;
    //NSLog(@"setPwm %f dir %d cn=%d", p, d, numc);
}


- (void) computeTrainsAfter:(NSTimeInterval)ellapsed sinceStart:(NSTimeInterval)ts
{
    if (ellapsed<20) {
        NSLog(@"ellapsed %f small", ellapsed);
    }
    for (int i=0; i<NUM_CANTONS; i++) {
        bemf[i]=0.0;
    }
    for (int tn=0; tn<NUM_TRAINS; tn++) {
        if (c1[tn]<0) continue;
        int cn = c1[tn];
        NSAssert(cn>=0, @"bad cn");
        NSAssert(cn<NUM_CANTONS, @"bad cn");
        const canton_config_t *cnf = get_canton_cnf(cn); // canton num / canton addr /local etc TODO
        
        // update pos
        position[tn] += dir[tn]*speed[tn]*ellapsed/1000;
        NSLog(@"xxxtrain %d pos: %f len %d", tn, position[tn], get_blk_len(cn));
        
        double spower = volt[cn]*pwm[cn]/1000.0;
        NSLog(@"train %d power %f", tn, spower);
        speed[tn] = spower * 25.0;
        double be = spower * 2.10 / 10.0;
        bemf[cn] = be * (cnf->reverse_bemf ? -1 : 1);
        int co = cold[tn];
        if (co>=0) {
            cnf = get_canton_cnf(co);
            bemf[co] = be * (cnf->reverse_bemf ? -1 : 1);
            cold[tn] = -1;
        }
    }
}

- (double) bemfForCantonNum:(int)numc
{
    // power to speed
    // no inertia after 20ms
    return bemf[numc];
}





@end
