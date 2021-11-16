//
//  cautoP.h
//  train_throttle
//
//  Created by Daniel BRAUN on 13/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef cautoP_h
#define cautoP_h

int cauto_update_turnouts(int tidx, lsblk_num_t cur, uint8_t left, uint8_t next);


void cauto_c1_updated(int tidx, train_ctrl_t *tvars);
void cauto_check_start(int tidx, train_ctrl_t *tvars);
void cauto_had_stop(int tidx, train_ctrl_t *tvars);
lsblk_num_t cauto_peek_next_lsblk(int tidx, train_ctrl_t *tvars, uint8_t left);
void cauto_had_trigU1(int tidx, train_ctrl_t *tvars);
void cauto_had_timer(int tidx, train_ctrl_t *tvars);
void cauto_end_tick(int tidx, train_ctrl_t *tvars);

// provided by ctrl.h
extern void ctrl2_set_turnout(int tn, int v); // ctrl.c TODO move proto
extern void ctrl2_send_led(uint8_t led_num, uint8_t prog_num);


/*
 0 0 x x x x x x   expected lsblk
 
 0 0 1 1 1 1 1 1   _AR_WSTOP
 
 
 1 0 s s s s s s   speed
 
 1 1 . . . . . .   ctrl
 
 1 1 0 0 0 x x x   wait event 0-7
 1 1 0 0 1 x x x   trig event 0-7
 1 1 0 1 0 t t t   set timer 2^t
 1 1 1 1 1 0 0 1   stop at half _AR_STPHALF
 1 1 1 1 1 0 1 0   wait timer _AR_WTIMER
 1 1 1 1 1 0 1 1   + led num + prog num : _AR_LED
 1 1 1 1 1 1 1 0   loop
 1 1 1 1 1 1 1 1   end
 */


#define _AR_SPD(_s) (0x80 | ((((int8_t)(_s))>>2) & 0x3F))
#define _AR_LOOP 0xFE
#define _AR_END  0xFF

#define _AR_WSTOP 0x3F  // special case of lsblk

#define _AR_STPHALF      (0xF9)
#define _AR_WEVENT(_e)   (0xC0 | ((_e) & 0x07))
#define _AR_TRGEVENT(_e) (0xC8 | ((_e) & 0x07))
#define _AR_TIMER(_t)    (0xD0 | ((_t) & 0x07))
#define _AR_WTIMER       (0xFA)
#define _AR_LED          (0xFB)
#endif /* cautoP_h */
