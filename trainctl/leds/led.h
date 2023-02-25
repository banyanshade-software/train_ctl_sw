//
//  led.h
//  train_throttle
//
//  Created by Daniel BRAUN on 20/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef led_h
#define led_h

#ifdef TRAIN_SIMU
#include <stdio.h>
#endif

void led_run_all(void);
void led_reset_all(void);
void led_start_prog(uint8_t lednum, uint8_t prognum);




#define LED_PRG_OFF 0
#define LED_PRG_25p 1
#define LED_PRG_50p 2
#define LED_PRG_ON  3
#define LED_PRG_OF2 4
#define LED_PRG_TST 5
#define LED_PRG_NEONON 6
#define LED_PRG_DIMOFF 7
#define LED_PRG_DIMON  8
#define LED_PRG_FLASH  9
#define LED_PRG_12p 10
#define LED_PRG_6p 11

//extern void led_io(uint8_t lednum, uint8_t v);

#endif /* led_h */
