//
//  oscilo.h
//  train_throttle
//
//  Created by Daniel BRAUN on 28/01/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef oscilo_h
#define oscilo_h


typedef struct {
    int16_t vadc[4]; // Va,Vb on Canton 0 and 1
    int16_t tim1cnt;
    int16_t tim2cnt;
    int16_t tim8cnt;
    uint8_t valt1ch1:1;
    uint8_t valt1ch2:1;
    uint8_t valt1ch3:1;
    uint8_t valt1ch4:1;
    uint8_t valt2ch1:1;
    uint8_t valt2ch2:1;
    uint8_t valt2ch3:1;
    uint8_t valt2ch4:1;
} osc_values_t;

#define OSC_NUM_SAMPLES 1024

extern int oscillo_trigger_start;

#endif /* oscilo_h */
