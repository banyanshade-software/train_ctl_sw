//
//  trig_tags.h
//  train_throttle
//
//  Created by Daniel Braun on 19/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef trig_tags_h
#define trig_tags_h


typedef enum pose_trig_tag {
    tag_invalid = 0,    // reserved. 0 means "no trigger"
    
    // tag_end_lsblk used when a canton is divided in sblk without specific current detection
    tag_end_lsblk = 1,
    
    tag_stop_blk_wait,  // stopped for occupency (eg before turnout)
    tag_stop_eot,       // stop for end of track
    
    tag_chkocc,     // rightcars will go into new segment, rightcars and leftcars need to be updated
    tag_brake,      // should start braking
    
    tag_free_left,
    tag_free_right, // TODO do we really need both ?
    
    tag_auto_u1,
} pose_trig_tag_t;

#endif /* trig_tags_h */
