//
//  SimTrain.m
//  train_throttle
//
//  Created by Daniel BRAUN on 10/10/2020.
//  Copyright © 2020 Daniel BRAUN. All rights reserved.
//

#import "SimTrain.h"

@interface SimTrain ()
@property (nonatomic,readwrite) double speed;
@end
@implementation SimTrain {
    double volt;
    double pwm;
    int dir;
    
    double power;
}

@synthesize speed = _speed;
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
    self.speed = s;
    // full speed : 1V
    double bemf = _speed / 5;
    //bemf += (rand()%100-50)*0.001; // more noise
    //bemf *= dir;
    return bemf;
}

- (int) simuCurCanton
{
    return 0;
}
- (int) simuNextCanton
{
    return -1;
}




@end
