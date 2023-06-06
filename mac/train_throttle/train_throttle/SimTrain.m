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
#include "conf_train.h"
#include "trace_train.h"

@interface SimTrain ()
//@property (nonatomic,readwrite) double speed;
@end


@implementation SimTrain {
    double volt[NUM_CANTONS];
    double pwm[NUM_CANTONS];
    int dir[NUM_CANTONS];
    double bemf[NUM_CANTONS];
    
    double speed[NUM_TRAINS];
    double positioncm[NUM_TRAINS];

    //int c1[NUM_TRAINS];
    lsblk_num_t s1[NUM_TRAINS];
    int cold[NUM_TRAINS];
}

@synthesize speed = _speed;

- (NSString *) htmlSimuStateForTrain:(int)tidx
{
    NSString *s = [NSString stringWithFormat:@"=== Train : %d<br>\n s1=%d pos=%.0fcm spd=%.0f<br>\n", tidx, s1[tidx].n, positioncm[tidx], speed[tidx]];
    for (int c=0; c<NUM_CANTONS; c++) {
        s = [s stringByAppendingFormat:@"  C%d : dir=%d vi=%.1f pwm=%.1f bemf=%.1f<br>\n", c, dir[c], volt[c], pwm[c], bemf[c] ];
    }
    return s;
}
- (instancetype) init
{
    self = [super init];
    if (!self) return nil;
    for (int i=0; i<NUM_TRAINS; i++) {
        s1[i].n = -1;
        cold[i] = -1;
        speed[i] = 0;
        positioncm[i] = 0;
    }
    for (int i=0; i<NUM_CANTONS; i++) {
        bemf[i] = 0.0;
        volt[i] = 2.0;
        pwm[i] = 0;
        dir[i] = 0;
    }
    /*s1[0].n = 5;                    // <<<<<<<<<<<<<<<
    position[0]= 0; //60;
     */
    return self;
}

- (void) setTrain:(int)tidx sblk:(int)sblk posmm:(int)posmm
{
    positioncm[tidx] = posmm/10;
    s1[tidx].n = sblk;
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


- (void) inaOff:(int)inanum train:(int)train
{
    msg_64_t m = {0};
    m.from = MA0_INA(inanum/12); // change to MA_INA3221_B ?
    m.to = MA1_CONTROL();
    m.cmd = CMD_PRESENCE_SUB_CHANGE;
    m.subc = inanum;
    m.v1u = 0;
    m.v2 = 0;//na_svalues[i];
    mqf_write_from_usb(&m);
}
- (void) inaOn:(int)inanum train:(int)train
{
    msg_64_t m = {0};
    m.from = MA0_INA(inanum/12); // change to MA_INA3221_B ?
    m.to = MA1_CONTROL();
    m.cmd = CMD_PRESENCE_SUB_CHANGE;
    m.subc = inanum;
    m.v1u = 1;
    m.v2 = 4000;//na_svalues[i];
    mqf_write_from_usb(&m);
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
        positioncm[tn] += speed[tn]*ellapsed/1000;
        //int get_lsblk_len(lsblk_num_t num);
        int blen = get_lsblk_len_cm(s1[tn], NULL);
        //NSLog(@"xxxtrain %d pos: %f len %d", tn, position[tn], get_lsblk_len(s1[tn], NULL));
        xblkaddr_t cn = canton_for_lsblk(s1[tn]);
        NSAssert(cn.v != 0xFF, @"bad cn");
        NSAssert(cn.v<NUM_CANTONS, @"bad cn");

        if (dir[cn.v]>0) {
            if (positioncm[tn]>blen) {
                lsblk_num_t ns = next_lsblk(s1[tn], 0, NULL);
                if (ns.n < 0) {
                    NSLog(@"END OF TRACK/COL !!");
                } else {
                    uint8_t inaold = get_lsblk_ina3221(s1[tn]);
                    uint8_t inanew = get_lsblk_ina3221(ns);
                    NSLog(@"switch ina %u->%u", inaold, inanew);
                    if (inanew != inaold) {
                        if (inaold != 0xFF) {
                            [self inaOff:inaold train:tn];
                        }
                        if (inanew != 0xFF) {
                            [self inaOn:inanew train:tn];
                        }
                    }
                    s1[tn] = ns;    //  >>>>>>>>>>> next sblk >>>>>>>>>>>>>
                    positioncm[tn] = 0;
                    xblkaddr_t nb = canton_for_lsblk(ns);
                    _trace_train_simu(SimuTick, tn, ns.n, nb.v);
                    
                    if (nb.v != cn.v) {
                        cold[tn] = cn.v;
                        cn = nb;
                    }
                }
            }
        } else if (dir[cn.v]<0) {
            if (positioncm[tn]<-blen) {
                lsblk_num_t ns = next_lsblk(s1[tn], 1, NULL);
                if (ns.n < 0) {
                    NSLog(@"END OF TRACK/COL !!");
                } else {
                    uint8_t inaold = get_lsblk_ina3221(s1[tn]);
                    uint8_t inanew = get_lsblk_ina3221(ns);
                    NSLog(@"switch ina %u->%u", inaold, inanew);
                    if (inanew != inaold) {
                        if (inaold != 0xFF) {
                            [self inaOff:inaold train:tn];
                        }
                        if (inanew != 0xFF) {
                            [self inaOn:inanew train:tn];
                        }
                    }
                    s1[tn] = ns; // <<<<<<<<<<< next sblk <<<<<<<<<<q
                    positioncm[tn] = 0;
                    xblkaddr_t nb = canton_for_lsblk(ns);
                    if (nb.v != cn.v) {
                        cold[tn] = cn.v;
                        cn = nb;
                    }
                }
            }
        }
        const conf_canton_t *cnf = conf_canton_get(cn.v); // canton num / canton addr /local etc TODO

        double spower = dir[cn.v]*volt[cn.v]*pwm[cn.v]/1000.0;
        //NSLog(@"train %d power %f", tn, spower);
        speed[tn] = spower * 50.0;
       
        const double multbemf = 0.8;  // empirical value that match real model
        double be =  multbemf * spower * 2.10 / 10.0;
        int revbemf = cnf->reverse_bemf;
        revbemf = 0;
        bemf[cn.v] = be * (revbemf ? -1 : 1);
        int co = cold[tn];
        if (co>=0) {
            cnf = conf_canton_get(co);
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

