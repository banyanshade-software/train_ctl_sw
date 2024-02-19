//
//  check_topology.c
//  SvgGen
//
//  Created by Daniel Braun on 16/02/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#include <stddef.h>

#ifdef TOPOLOGY_SVG
#define TRACKPLAN_TESTPGM
#define BOARD_HAS_TOPOLOGY
#include "boards_def.h"
#endif

//#include "../misc.h"

#ifndef TRACKPLAN_TESTPGM
#include "../msg/trainmsg.h"
#include "../railconfig.h"
#else
#include <stdio.h>
#include <stdlib.h>

#define DBG_TURNOUTS 1
//#define itm_debug3(_fl, _msg, _a, _b, _c) do {printf(_msg  "%d %d %d", _a, _b, _c);} while(0)
#endif


#ifndef BOARD_HAS_TOPOLOGY
#error BOARD_HAS_TOPOLOGY not defined, remove this file from build
#endif


//#include "topology.h"
#include "topologyP.h"
#include "../config/conf_topology.h"

#include "check_topology.h"

#define FUTURE_CANTON 0xFF


static int check_right_tn(int8_t lsblk, uint8_t tn, uint8_t expright, int nright)
{
    const conf_topology_t *s = conf_topology_get(lsblk);
    if (s->rtn != tn) return check_left_has_bad_rtn;
    if ((s->right1 != expright) && (s->right2 != expright)) return check_left_bad_right_lsb;
    switch (nright) {
        default: //FALLTHRU
        case -1:
            return 0;
        case 1:
            if ((s->right2 != -1) && (s->right1 != -1)) {
                return check_left_has_two;
            }
            break;
        /*case 2:
            if (s->right2 == -1) return -8;
            break; */
    }
    return 0;
}
static int check_left_tn(int8_t lsblk, uint8_t tn, uint8_t expleft, int nleft)
{
    const conf_topology_t *s = conf_topology_get(lsblk);
    if (s->ltn != tn) return check_right_has_bad_ltn;
    if ((s->left1 != expleft) && (s->left2 != expleft)) return check_right_bad_left_lsb;
    switch (nleft) {
        default: //FALLTHRU
        case -1:
            return 0;
        case 1:
            if ((s->left2 != -1) && (s->left1 != -1)) {
                return check_right_has_two;
            }
            break;
        /*case 2:
            if (s->right2 == -1) return -8;
            break; */
    }
    return 0;
}


topology_check_rc_t topology_check(int *plsblk, int *secsblk)
{
    int n = conf_topology_num_entries();
    for (int i = 0; i<n; i++) {
        const conf_topology_t *s = conf_topology_get(i);
        if (!s) {
            break;
        };
        if (plsblk) *plsblk = i;
        int rc;
        
        // LEFT
        *secsblk = -1;
        if ((s->left1 != -1) && (s->left2 != -1)) {
            if (s->ltn == 0xFF) {
                return check_no_ltn;
            }
            *secsblk = s->left1;
            rc = check_right_tn(s->left1, s->ltn, i, 1);
            if (rc) {
                return rc;
            }
            *secsblk = s->left2;
            rc = check_right_tn(s->left2, s->ltn, i, 1);
            if (rc) {
                return rc;
            }
        } else if (s->left1 != -1) {
            *secsblk = s->left1;
            if ((s->ltn != 0xFF) && (s->left2 != -1)) {
                rc = check_right_tn(s->left1, s->ltn, i, 1);
            } else {
                rc = check_right_tn(s->left1, s->ltn, i, -1);
            }
            if (rc) {
                return rc;
            }
        } else if (s->left2 != -1) {
            *secsblk = s->left2;
            if ((s->ltn != 0xFF) && (s->left1 != -1)) {
                rc = check_right_tn(s->left2, s->ltn, i, 1);
            } else {
                rc = check_right_tn(s->left2, s->ltn, i, -1);
            }
            if (rc) {
                return rc;
            }

        } else {
            // no left
            if (s->ltn != 0xFF) {
                return check_no_left_but_ltn;
            }
        }
        
        
        // right
        *secsblk = -1;
        if ((s->right1 != -1) && (s->right2 != -1)) {
            if (s->rtn == 0xFF) {
                return check_no_rtn;
            }
            *secsblk = s->right1;
            rc = check_left_tn(s->right1, s->rtn, i, 1);
            if (rc) {
                return rc;
            }
            *secsblk = s->right2;
            rc = check_left_tn(s->right2, s->rtn, i, 1);
            if (rc) {
                return rc;
            }
        } else if (s->right1 != -1) {
            *secsblk = s->right1;
            if ((s->rtn != 0xFF) && (s->right2 != -1)) {
                rc = check_left_tn(s->right1, s->rtn, i, 1);
            } else {
                rc = check_left_tn(s->right1, s->rtn, i, -1);
            }
            if (rc) {
                return rc;
            }
        } else if (s->right2 != -1) {
            *secsblk = s->right2;
            if ((s->rtn != 0xFF) && (s->right1 != -1)) {
                rc = check_left_tn(s->right2, s->rtn, i, 1);
            } else {
                rc = check_left_tn(s->right2, s->rtn, i, -1);
            }
            if (rc) {
                return rc;
            }
        } else {
            // no right
            if (s->rtn != 0xFF) {
                return check_no_right_but_rtn;
            }
        }
    }
    return 0;
}
