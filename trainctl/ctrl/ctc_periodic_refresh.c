//
//  ctc_periodic_refresh.c
//  train_throttle
//
//  Created by Daniel Braun on 12/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#include "ctc_periodic_refresh.h"



#include "../misc.h"
#include "../msg/trainmsg.h"
#include "ctrl.h"
#include "../topology/topology.h"

#include "ctc_periodic_refresh.h"

#include "../topology/topologyP.h"
#include "../topology/occupency.h"
#include "../config/conf_canton.h"

static void refresh_ctc_turnout(xtrnaddr_t tn)
{
    if (tn.v == 0xFF) return;
    enum topo_turnout_state v = topology_get_turnout(tn);
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA3_UI_CTC;
    m.subc = tn.v;
    m.cmd = CMD_TN_CHG_NOTIF;
    m.v1 = v;
    mqf_write_from_ctrl(&m);
}


static void occupency_periodic_refresh(void)
{
    static int n=0;
    int nc = conf_canton_num_entries();
    xblkaddr_t b;
    b.v = n;
    uint8_t train;
    uint8_t sblk;
    int occ = occupency_block_addr_info(b, &train, &sblk);
    
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA3_UI_CTC;
    m.cmd = CMD_BLK_CHG_NOTIF;
    m.vbytes[0] = b.v;
    m.vbytes[1] = occ;
    m.vbytes[2] = train;
    m.vbytes[3] = sblk;
    mqf_write_from_ctrl(&m);
    
    n++;
    if (n>= nc) n = 0;
}
void ctc_periodic_refresh(uint32_t tick)
{
    static uint32_t ltick = 0;
    if (tick-ltick<300) return;
    ltick = tick;
    
    occupency_periodic_refresh();
    static int c = 0;
    static int l = 0;
    int n = topology_num_sblkd();
    
    if (c>=n) c = 0;
    
    const topo_lsblk_t *topo = topology_get_sblkd(c);
    xtrnaddr_t tn;
    if (l) {
        if ((topo->canton_addr != 0xFF) && (topo->ltn != 0xFF)) {
            tn.v = topo->ltn;
            refresh_ctc_turnout(tn);
        }
        l = 1;
    } else {
        if ((topo->canton_addr != 0xFF) && (topo->rtn != 0xFF)) {
            tn.v = topo->rtn;
            refresh_ctc_turnout(tn);
        }
        l = 0;
        c = c+1;
    }
}
