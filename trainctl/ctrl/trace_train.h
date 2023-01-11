//
//  trace_train.h
//  train_throttle
//
//  Created by Daniel Braun on 11/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef trace_train_h
#define trace_train_h



#include <stdint.h>

#include "trig_tags.h"
#include "ctrl.h"
#include "ctrlLT.h"

#define trace_train_enable 1

void _trace_train_postick(uint32_t tick, int tidx, train_ctrl_t *tvars);
static inline void trace_train_postick(uint32_t tick, int tidx, train_ctrl_t *tvars)
{
    if (!trace_train_enable) return;
    _trace_train_postick(tick, tidx, tvars);
}


void _trace_train_trig(uint32_t tick, int tidx, train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t pos);
static inline void trace_train_trig(uint32_t tick, int tidx, train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t pos)
{
    if (!trace_train_enable) return;
    _trace_train_trig(tick, tidx, tvars, tag, pos);
}

void _trace_train_trig_set(uint32_t tick, int tidx, train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t pos);
static inline void trace_train_trig_set(uint32_t tick, int tidx, train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t pos)
{
    if (!trace_train_enable) return;
    _trace_train_trig_set(tick, tidx, tvars, tag, pos);
}



void trace_train_dump(int tidx);

#endif /* trace_train_h */
