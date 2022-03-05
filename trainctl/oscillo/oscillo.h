//
//  oscillo.h
//  train_throttle
//
//  Created by Daniel BRAUN on 28/01/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef oscillo_h
#define oscillo_h


typedef struct {
    int16_t vadc[4]; // Va,Vb on Canton 0 and 1
    int16_t tim1cnt;
    int16_t tim2cnt;
    int16_t tim8cnt;

    int16_t t0bemf;
    int16_t t1bemf;

    uint8_t valt1ch1:1;
    uint8_t valt1ch2:1;
    uint8_t valt1ch3:1;
    uint8_t valt1ch4:1;

    uint8_t valt2ch1:1;
    uint8_t valt2ch2:1;
    uint8_t valt2ch3:1;
    uint8_t valt2ch4:1;

    uint8_t evtadc:4;

    int16_t ina0;
    int16_t ina1;
    int16_t ina2;
} osc_values_t;

#define OSC_NUM_SAMPLES 1024

extern volatile int oscillo_trigger_start;

extern volatile int16_t oscillo_t0bemf;
extern volatile int16_t oscillo_t1bemf;
extern volatile uint8_t oscillo_evtadc;

extern volatile int ocillo_enable;

extern volatile int16_t oscillo_ina0;
extern volatile int16_t oscillo_ina1;
extern volatile int16_t oscillo_ina2;

#endif /* oscilo_h */
