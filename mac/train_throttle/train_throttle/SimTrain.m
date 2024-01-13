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
    
    double _positioncm[NUM_TRAINS]; // position in current sblk
    lsblk_num_t _s1[NUM_TRAINS]; // curresnt sblk
    int cold[NUM_TRAINS];       // old canton
}

@synthesize speed = _speed;

- (NSString *) htmlSimuStateForTrain:(int)tidx
{
    NSString *s = [NSString stringWithFormat:@"=== Train : %d<br>\n s1=%d pos=%.0fcm spd=%.0f<br>\n", tidx, _s1[tidx].n, _positioncm[tidx], speed[tidx]];
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
        cold[i] = -1;
        speed[i] = 0;
        [self setTrain:i sblk:-1 posmm:0];
        //s1[i].n = -1;
        //positioncm[i] = 0;
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
    _positioncm[tidx] = posmm/10;
    _s1[tidx].n = sblk;
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


static uint16_t ina_detect_bitfield = 0;

- (void) computeTrainsAfter:(uint32_t)ellapsed sinceStart:(uint32_t)ts
{
    //if (ellapsed<20) {
    //    NSLog(@"ellapsed %f small", ellapsed);
    //}
    for (int i=0; i<NUM_CANTONS; i++) {
        bemf[i]=0.0;
    }
    for (int tn=0; tn<NUM_TRAINS; tn++) {
        if (_s1[tn].n < 0) continue;
        //int cn = c1[tn];
        
        // handle detection
        if (ina_detect_bitfield) {
            ina_num_t ina = get_lsblk_ina3221(_s1[tn]);
            if (ina_detect_bitfield & (1U<<ina.ina)) {
                // detected
                msg_64_t m = {0};
                m.from = MA0_INA(oam_localBoardNum());
                m.to = MA1_CONTROL();
                m.cmd = CMD_DETECTION_REPORT;
                m.subc = ina.ina;
                m.v1 = 1; // not realistic
                mqf_write_from_ina3221(&m);
                // clear bit in ina_detect_bitfield so detection is performed once only
                ina_detect_bitfield &= ~(1U<<ina.v);
            }
            // assume train doesn't move during detection
            return;
        }
        // update pos
        [self setTrain:tn sblk:_s1[tn].n posmm:_positioncm[tn]+speed[tn]*ellapsed/1000];
        //positioncm[tn] += speed[tn]*ellapsed/1000;
        //int get_lsblk_len(lsblk_num_t num);
        int blen = get_lsblk_len_cm(_s1[tn], NULL);
        //NSLog(@"xxxtrain %d pos: %f len %d", tn, position[tn], get_lsblk_len(s1[tn], NULL));
        xblkaddr_t cn = canton_for_lsblk(_s1[tn]);
        NSAssert(cn.v != 0xFF, @"bad cn");
        NSAssert(cn.v<NUM_CANTONS, @"bad cn");

        if (dir[cn.v]>0) {
            if (_positioncm[tn]>blen) {
                lsblk_num_t ns = next_lsblk(_s1[tn], 0, NULL);
                if (ns.n < 0) {
                    NSLog(@"END OF TRACK/COL !!");
                } else {
                    ina_num_t inaold = get_lsblk_ina3221(_s1[tn]);
                    ina_num_t inanew = get_lsblk_ina3221(ns);
                    NSLog(@"switch ina %u->%u", inaold.v, inanew.v);
                    if (inanew.v != inaold.v) {
                        if (inaold.v != 0xFF) {
                            [self inaOff:inaold.v train:tn];
                        }
                        if (inanew.v != 0xFF) {
                            [self inaOn:inanew.v train:tn];
                        }
                    }
                    [self setTrain:tn sblk:ns.n posmm:0];
                    //s1[tn] = ns;    //  >>>>>>>>>>> next sblk >>>>>>>>>>>>>
                    //positioncm[tn] = 0;
                    xblkaddr_t nb = canton_for_lsblk(ns);
                    _trace_train_simu(SimuTick, tn, ns.n, nb.v);
                    
                    if (nb.v != cn.v) {
                        cold[tn] = cn.v;
                        cn = nb;
                    }
                }
            }
        } else if (dir[cn.v]<0) {
            if (_positioncm[tn]<-blen) {
                lsblk_num_t ns = next_lsblk(_s1[tn], 1, NULL);
                if (ns.n < 0) {
                    NSLog(@"END OF TRACK/COL !!");
                } else {
                    ina_num_t inaold = get_lsblk_ina3221(_s1[tn]);
                    ina_num_t inanew = get_lsblk_ina3221(ns);
                    NSLog(@"switch ina %u->%u", inaold, inanew);
                    if (inanew.v != inaold.v) {
                        if (inaold.v != 0xFF) {
                            [self inaOff:inaold.v train:tn];
                        }
                        if (inanew.v != 0xFF) {
                            [self inaOn:inanew.v train:tn];
                        }
                    }
                    [self setTrain:tn sblk:ns.n posmm:0];
                    //s1[tn] = ns; // <<<<<<<<<<< next sblk <<<<<<<<<<q
                    //positioncm[tn] = 0;
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

/*
 for simu, we run msg read loop, just to avoid ina msg queue to be full, and
 for ina based detection
 */


void ina_simu_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
    for (;;) {
        msg_64_t m;
        int rc = mqf_read_to_ina3221(&m);
        if (rc) break;
        switch (m.cmd) {
            case CMD_START_INA_MONITOR:
                itm_debug1(DBG_DETECT, "START monitor", m.va16);
                ina_detect_bitfield = m.va16;
                break;
                
            case CMD_SETRUN_MODE:
                ina_detect_bitfield = 0;
                break;
                
            default:
                itm_debug1(DBG_DETECT, "msg", m.cmd);
                break;
        }
    }
}
