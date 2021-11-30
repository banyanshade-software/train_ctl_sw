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

    //int c1[NUM_TRAINS];
    lsblk_num_t s1[NUM_TRAINS];
    int cold[NUM_TRAINS];
}

@synthesize speed = _speed;

- (instancetype) init
{
    self = [super init];
    if (!self) return nil;
    for (int i=0; i<NUM_TRAINS; i++) {
        s1[i].n = -1;
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
    s1[0].n = 2;
    position[0]=60;
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


- (void) computeTrainsAfter:(uint32_t)ellapsed sinceStart:(uint32_t)ts
{
    //if (ellapsed<20) {
    //    NSLog(@"ellapsed %f small", ellapsed);
    //}
    for (int i=0; i<NUM_CANTONS; i++) {
        bemf[i]=0.0;
    }
    for (int tn=0; tn<NUM_TRAINS; tn++) {
        if (s1[tn].n < 0) continue;
        //int cn = c1[tn];
        
        // update pos
        position[tn] += speed[tn]*ellapsed/1000;
        //int get_lsblk_len(lsblk_num_t num);
        int blen = get_lsblk_len(s1[tn], NULL);
        NSLog(@"xxxtrain %d pos: %f len %d", tn, position[tn], get_lsblk_len(s1[tn], NULL));
        int cn = canton_for_lsblk(s1[tn]);
        NSAssert(cn>=0, @"bad cn");
        NSAssert(cn<NUM_CANTONS, @"bad cn");

        if (dir[cn]>0) {
            if (position[tn]>blen) {
                lsblk_num_t ns = next_lsblk(s1[tn], 0, NULL);
                if (ns.n < 0) {
                    NSLog(@"END OF TRACK/COL !!");
                } else {
                    s1[tn] = ns;
                    position[tn] = 0;
                    uint8_t nb = canton_for_lsblk(ns);
                    if (nb != cn) {
                        cold[tn] = cn;
                        cn = nb;
                    }
                }
            }
        } else if (dir[cn]<0) {
            if (position[tn]<-blen) {
                lsblk_num_t ns = next_lsblk(s1[tn], 1, NULL);
                if (ns.n < 0) {
                    NSLog(@"END OF TRACK/COL !!");
                } else {
                    s1[tn] = ns;
                    position[tn] = 0;
                    uint8_t nb = canton_for_lsblk(ns);
                    if (nb != cn) {
                        cold[tn] = cn;
                        cn = nb;
                    }
                }
            }
        }
        const canton_config_t *cnf = get_canton_cnf(cn); // canton num / canton addr /local etc TODO

        double spower = dir[cn]*volt[cn]*pwm[cn]/1000.0;
        NSLog(@"train %d power %f", tn, spower);
        speed[tn] = spower * 50.0;
        double be = spower * 2.10 / 10.0;
        int revbemf = cnf->reverse_bemf;
        revbemf = 0;
        bemf[cn] = be * (revbemf ? -1 : 1);
        int co = cold[tn];
        if (co>=0) {
            cnf = get_canton_cnf(co);
            bemf[co] = be * (revbemf ? -1 : 1);
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
