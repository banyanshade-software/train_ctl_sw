//
//  trig_tags.h
//  train_throttle
//
//  Created by Daniel Braun on 19/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef trig_tags_h
#define trig_tags_h

/*
 trigger tag identify cause of trigger, and possible actions
 values must feet on 7 bits, leaving 127 possible values
 */
typedef enum pose_trig_tag {
    tag_invalid = 0,    // reserved. 0 means "no trigger"
    
    // tag_end_lsblk used when a canton is divided in sblk without specific current detection
    tag_end_lsblk = 1,
    tag_need_c2,        // C2 needs to be powered
    tag_reserve_c2,     // C2 needs to be occupied
    
    tag_stop_blk_wait,  // stopped for occupency (eg before turnout)
    tag_stop_eot,       // stop for end of track
    
    tag_chkocc,     // rightcars will go into new segment, rightcars and leftcars need to be updated
    tag_brake,      // should start braking
    tag_brake_user, // brake triggered by cauto. really the same than tag_brake
                    // but we want to see it on traces
    
    tag_free_back,  // cars on the rear leaving a block
    tag_leave_canton,
    
    tag_auto_u1,
} __attribute__((packed)) pose_trig_tag_t;

#endif /* trig_tags_h */
