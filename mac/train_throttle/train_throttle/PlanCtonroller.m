//
//  PlanCtonroller.m
//  train_throttle
//
//  Created by Daniel Braun on 27/09/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#import "PlanCtonroller.h"
#import "PlanCellView.h"
#import "PlanCellController.h"
#import "AppDelegate.h"
#import "AppDelegateP.h"

#include "topology.h"

#define NUM_PLAN_TRAIN 4

@implementation PlanCtonroller {
    PlanCellController __strong *tcellCtrl[NUM_PLAN_TRAIN];
    //NSMutableArray <PlanCellController *> *tcellCtrl;
    PlanCellController *tst;
}

@synthesize tableview = _tableview;
@synthesize startDelay = _startDelay;


- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
   return NUM_PLAN_TRAIN+1;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        memset(tcellCtrl, 0, sizeof(tcellCtrl));
    }
  
    return self;
}
- (void)awakeFromNib
{
    NSNib *plancellNib = [[NSNib alloc] initWithNibNamed:@"plancell" bundle:nil];
    [_tableview registerNib:plancellNib forIdentifier:@"plancell"];
    NSNib *commitcellNib = [[NSNib alloc] initWithNibNamed:@"plancommit" bundle:nil];
    [_tableview registerNib:commitcellNib forIdentifier:@"plancommit"];
    self.startDelay = 5;
    //tcellCtrl = [[NSMutableArray alloc]initWithCapacity:NUM_PLAN_TRAIN];
}
- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row
{
    NSString *cid = (row<NUM_PLAN_TRAIN) ? @"plancell" : @"plancommit";
     
    NSTableCellView *result = [tableView makeViewWithIdentifier:cid owner:self];
    if (row<NUM_PLAN_TRAIN) {
        PlanCellView *cv = (PlanCellView *)result;
        PlanCellController *c = cv.controller;
        NSAssert(c, @"no view controller");
        c.trainNum = row;
        tcellCtrl[row] = c;
        if (0==row) tst = c;
        if (0==row) {
            tcellCtrl[row].trainNum = 0;
            tcellCtrl[row].spd      = 72;
            tcellCtrl[row].targetLSBLK = 9;
        }
    }
    //result.autoresizingMask = NSViewHeightSizable ;
    return result;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    if (row<NUM_PLAN_TRAIN) return 100;
    return 128;
}



- (IBAction) resetAll:(id)sender
{
    //NSInteger c = [tcellCtrl count];
    //NSAssert(c==NUM_PLAN_TRAIN, @"bad count");
    for (NSInteger i = 0; i<NUM_PLAN_TRAIN; i++) {
        [tcellCtrl[i] resetEntry];
    }
}
- (IBAction) startAllDelay:(id)sender
{
    [self startAllAfter:(int)self.startDelay];
}
- (IBAction) startAllNow:(id)sender
{
    [self startAllAfter:0];
}

- (void) startAllAfter:(int)delay
{
    int nrule = 0;
    msg_64_t m = {0};
    m.from = MA3_UI_CTC;
    m.to = MA3_PLANNER;
    m.cmd = CMD_PLANNER_CANCEL;
    [theDelegate sendMsg64:m];
    
    for (int i=0; i<NUM_PLAN_TRAIN; i++) {
        PlanCellController *c = tcellCtrl[i];
        if (!c.hasRule) continue;
        NSLog(@"train %d after %d go %d\n", (int) c.trainNum, (int) c.delay, (int) c.targetLSBLK);
        m.cmd = CMD_PLANNER_ADD;
        m.subc = c.trainNum;
        m.vb8 = c.targetLSBLK;
        m.va16 = c.delay;
        m.vcu8 = c.spd;
        [theDelegate sendMsg64:m];
        nrule++;
    }
    if (!nrule) return;
    m.cmd = CMD_PLANNER_COMMIT;
    m.v1u = delay;
    m.v2 = 0;
    [theDelegate sendMsg64:m];
}
@end
