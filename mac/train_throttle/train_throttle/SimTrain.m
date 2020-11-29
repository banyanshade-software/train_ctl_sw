//
//  SimTrain.m
//  train_throttle
//
//  Created by Daniel BRAUN on 10/10/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "SimTrain.h"
#import "railconfig.h"

@interface SimTrain ()
@property (nonatomic,readwrite) double speed;
@end
@implementation SimTrain {
    double volt;
    double pwm;
    int dir;
    
    double power;
    
    double position;
    double len;
    double c1_len;
    int c0;
    int c1;
}

@synthesize speed = _speed;

- (instancetype) init
{
    self = [super init];
    if (!self) return nil;
    c0 = 0;
    c1 = 0xFF;
    position = 0;
    const block_canton_config_t *bcnf = get_block_canton_cnf(c0);
    len = bcnf->len;
    return self;
}

- (void)setVolt:(double)v
{
    volt = v;
    [self updatePower];
}
- (void)setPwm:(double)p dir:(int)d
{
    pwm = p;
    dir = d;
    
    [self updatePower];
}

- (void) updatePower
{
    power = volt*pwm/1000.0; // 0-1
}

- (double) bemfAfter:(NSTimeInterval)ellapsed sinceStart:(NSTimeInterval)ts
{
    // power to speed
    // no inertia after 20ms

    double s; // 0 -1
    if (power<0.30) s = 0;
    else s = dir*(power-0.30);
    if (ellapsed<20) {
        s = (ellapsed*s - (20-ellapsed)*_speed)/20;
    }
    //s += ((rand()%100)-50)*0.001;
    if (ts>10) {
        s = s*.7; // steep for instance
    }
    position += s;
    self.speed = s;
    // full speed : 1V
    double bemf = _speed / 5;
    //bemf += (rand()%100-50)*0.001; // more noise
    //bemf *= dir;
    
    int d=0;
    double delta=-1;
    if (position<0) {
        //NSLog(@"neg pos");
        if (s>0) {
            NSLog(@"but speed>0 ?");
        }
        d = -1;
        delta = -position;
    } else if (position>=len) {
        if (s<0) {
            NSLog(@"but speed<0 ?");
        }
        d = 1;
        delta = position-len;
    }
    if (d) {
        NSAssert(delta>=0, @"holala");
        if (c1==0xFF) {
            [self findC1:d];
            if (c1==0xFF) {
                NSLog(@"end of track");
                position = (d>=0) ? len : 0;
            }
        } else if (delta>0.9) {
            c0 = c1;
            c1 = 0xFF;
            position = 0;
            len = 0;
            if (c0 != 0xFF) {
                const block_canton_config_t *bcnf = get_block_canton_cnf(c0);
                len = bcnf->len;
                position = (d>=0) ? 0 : len;
            }
        }
    }
    return bemf;
}

- (void) findC1:(int)dir
{
    if (dir>0) {
        switch (c0) {
            case 0: c1 = 2; break;
            case 1: c1 = 2; break;
            default: c1 = 0xFF; break;
        }
    } else {
        switch (c0) {
            case 0: c1 = 0xFF; break;
            case 1: c1 = 0xFF; break;
            default: c1 = 1; break;
        }    }
}

- (int) simuCurCanton
{
    return (c0==0xFF) ? -1 : c0;
}
- (int) simuNextCanton
{
    return (c1==0xFF) ? -1 : c1;
}




@end
