//
//  AppDelegate.m
//  trackplan2
//
//  Created by Daniel BRAUN on 04/12/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#import "AppDelegate.h"
#import "CTCManager.h"
#include "topology.h"
#include "model.h"
#include "agentQ.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate {
    int curst[4];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    [_ctcManager loadHtml];
    self.T0_from = 0; self.T0_to = 7;
    self.T1_from = 2; self.T1_to = 8;
    self.T2_from = self.T2_to = self.T3_to = self.T3_from = -1;
    
    self.q_alpha = 0.1;
    self.q_gamma = 0.9;
    self.q_epsilon = 0.1;
    self.q_noise = 0.001;
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


int occupency_turnout_reserve(uint8_t turnout, int8_t numtrain)
{
    return 0;
}


- (void) placeOrigins
{
    [_ctcManager hideTrainInfos];
    if (_T0_from >= 0) {
        [self move:0 from:_T0_from to:_T0_from];
    }
    if (_T1_from >= 0) {
        [self move:1 from:_T1_from to:_T1_from];
        
    }
    if (_T2_from >= 0) {
        [self move:2 from:_T2_from to:_T2_from];
        
    }
    if (_T3_from >= 0) {
        [self move:2 from:_T2_from to:_T2_from];
    }
}

- (void) move:(int)train from:(int)oldblk to:(int)newblk
{
    
    //- (void) uitrac_change_sblk:(int) sblk val:(int)v train:(int)trn
    [_ctcManager uitrac_change_sblk:oldblk val:0 train:train];
    [_ctcManager uitrac_change_sblk:newblk val:1 train:train];

    //[_ctcManager uitrac_change_blk:canton_for_lsblk(oldlsb) val:0 train:0xFF sblk:-1];
    //[_ctcManager uitrac_change_blk:canton_for_lsblk(newlsb) val:1 train:train sblk:newblk ];
}

- (void) setupModel
{
    [self placeOrigins];
    int nt = 4;
    if (_T3_from<0) nt=3;
    if (_T2_from<0) nt=2;
    if (_T1_from<0) nt=1;
    if (_T0_from<0) return;
    
    model_setup(nt);
    [self setupModelDest];
}


- (void) setupModelDest
{
    model_set_from_to(0, _T0_from, _T0_to);
    if (_T1_from>=0) model_set_from_to(1, _T1_from, _T1_to);
    if (_T2_from>=0) model_set_from_to(2, _T2_from, _T2_to);
    if (_T3_from>=0) model_set_from_to(3, _T3_from, _T3_to);
}
- (IBAction) initQ:(id)sender
{
    [self setupModel];
    int ns = model_num_states();
    NSLog(@"num states : %d\n", ns);
    int na = model_num_actions();
    NSLog(@"num actions : %d\n", na);
    NSLog(@"q matrix : %d x %d = %d (%d bytes)", ns, na, ns*na, ns*na*4);
    
    int s = model_initial_state();
    int p0, p1, p2, p3;
    model_positions_for_state(s, &p0, &p1, &p2, &p3);
    NSLog(@"..");
    
    agentq_init();
}


- (IBAction) runQ:(id)sender
{
    if ((1)) {
        for (int i=0; i<10; i++) {
            BOOL ok = [self _runQ];
            if (ok) break;
        }
    } else {
        [self _runQdisplay];
    }
    if ((0)) {
        int st = model_initial_state();
        int act = 0;
        for (int i=0; i<1; i++) {
            q_dump_state(st);
            printf("act %d -> ", act);
            int badmove; double reward;
            st = model_new_state(st, act, &reward, &badmove);
            printf(" st%d (%sreward %2.2f\n", st, badmove ? "BAD ":"", reward);
        }
    }
}
- (IBAction) runQanim:(id)sender
{
    [self placeOrigins];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self _runQdisplay];
    });
}
- (BOOL) _runQ
{
    [self setupModelDest];
    agentq_restart();
    agentq_setparams(_q_alpha, _q_gamma, _q_epsilon, _q_noise);
    int i;
    int done=0;
    int nm=0;
    int nact = 0;
    int ps = model_initial_state();
    for (i=0; i<80000; i++) {
        int ns;
        done = q_step(&ns);
        
        if (ns==ps) {
            continue;
        }
        ps=ns;
        nact++;
        
        // display
        if ((0)) {
            int t[4];
            model_positions_for_state(ns, &t[0], &t[1], &t[2], &t[3]);
            for (int i=0; i<4; i++) {
                if ((0) || (t[i] != curst[i])) {
                    [self move:i from:curst[i] to:t[i]];
                    curst[i] = t[i];
                    nm++;
                }
            }
        }
        if (done) {
            break;
        }
    }
    NSLog(@"done %d (i=%d, nact %d nm %d)", done, i, nact, nm);
    if (done) {
        NSLog(@"youpla");
        return YES;
    }
    return NO;
}

- (void) _runQdisplay
{
    agentq_restart();
    agentq_setparams(_q_alpha, _q_gamma, _q_epsilon, _q_noise);
    
    __block int done = 0;
    
    __block void(^stepnext)(int);
    
    void (^stepblk)(int) = ^(int iter){
        if (iter>80000) {
            NSLog(@"fail iter=%d", iter);
            return;
        }
        int ns;
        int ok = q_step(&ns);
        if ((1)) {
            int t[4];
            model_positions_for_state(ns, &t[0], &t[1], &t[2], &t[3]);
            for (int i=0; i<4; i++) {
                if (t[i] != self->curst[i]) {
                    [self move:i from:self->curst[i] to:t[i]];
                    self->curst[i] = t[i];
                }
            }
        }
        if (ok) {
            NSLog(@"done iter=%d", iter);
            done = 1;
            return;
        }
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(500 * NSEC_PER_MSEC)), dispatch_get_main_queue(), ^{
            stepnext(iter+1);
        });
    };
    stepnext = stepblk;
    
    stepblk(1);
    
}

@end
