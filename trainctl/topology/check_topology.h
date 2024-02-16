//
//  check_topology.h
//  SvgGen
//
//  Created by Daniel Braun on 16/02/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#ifndef check_topology_h
#define check_topology_h

#include <stdint.h>

typedef enum {
    check_ok=0,
    check_no_ltn,
    check_no_left_but_ltn,
    check_left_has_bad_rtn,
    check_left_bad_right_lsb,
    check_left_has_two,
    
    check_no_rtn,
    check_no_right_but_rtn,
    check_right_has_bad_ltn,
    check_right_bad_left_lsb,
    check_right_has_two,
} topology_check_rc_t;

topology_check_rc_t topology_check(int *plsblk, int *secsblk);

#endif /* check_topology_h */
