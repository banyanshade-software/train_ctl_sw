// this file is automatically generated
#include <stdint.h>
#include <stddef.h>

#include "propag.h"

#include "conf_utestloc.h"
#include "conf_utestloc.propag.h"
#include "conf_utest.h"
#include "conf_utest.propag.h"
#include "conf_globparam.h"
#include "conf_globparam.propag.h"
#include "conf_led.h"
#include "conf_led.propag.h"
#include "conf_turnout.h"
#include "conf_turnout.propag.h"
#include "conf_train.h"
#include "conf_train.propag.h"
#include "conf_topology.h"
#include "conf_topology.propag.h"
#include "conf_canton.h"
#include "conf_canton.propag.h"
#include "conf_boards.h"
#include "conf_boards.propag.h"


void conf_propagate(unsigned int confnum, unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    switch (confnum) {
    case conf_pnum_utest:
        conf_utest_propagate(instnum, fieldnum, v);
        break;
    case conf_pnum_led:
        conf_led_propagate(instnum, fieldnum, v);
        break;
    case conf_pnum_turnout:
        conf_turnout_propagate(instnum, fieldnum, v);
        break;
    case conf_pnum_canton:
        conf_canton_propagate(instnum, fieldnum, v);
        break;
    default: break;
    }

}



int32_t conf_default_value(unsigned int confnum, unsigned int fieldnum, unsigned int board, unsigned int instnum)
{
    switch (confnum) {
    case conf_pnum_utest:
        return conf_utest_default_value(instnum, fieldnum, board);
        break;
    case conf_pnum_led:
        return conf_led_default_value(instnum, fieldnum, board);
        break;
    case conf_pnum_turnout:
        return conf_turnout_default_value(instnum, fieldnum, board);
        break;
    case conf_pnum_canton:
        return conf_canton_default_value(instnum, fieldnum, board);
        break;
    default: return 0;
    break;
    }

}



void *conf_local_ptr(unsigned int lconfnum)
{
    switch (lconfnum) {
    case conf_lnum_utestloc:
       return conf_utestloc_ptr();
    case conf_lnum_globparam:
       return conf_globparam_ptr();
    case conf_lnum_train:
       return conf_train_ptr();
    case conf_lnum_topology:
       return conf_topology_ptr();
    case conf_lnum_boards:
       return conf_boards_ptr();
    }
    return NULL;
}




unsigned int conf_local_size(unsigned int lconfnum)
{
    switch (lconfnum) {
    case conf_lnum_utestloc:
       return sizeof(conf_utestloc_t)*conf_utestloc_num_entries();
    case conf_lnum_globparam:
       return sizeof(conf_globparam_t)*conf_globparam_num_entries();
    case conf_lnum_train:
       return sizeof(conf_train_t)*conf_train_num_entries();
    case conf_lnum_topology:
       return sizeof(conf_topology_t)*conf_topology_num_entries();
    case conf_lnum_boards:
       return sizeof(conf_boards_t)*conf_boards_num_entries();
    }
    return 0;
}




int32_t conf_local_get(unsigned int lconfnum, unsigned int fieldnum, unsigned int instnum)
{
    switch (lconfnum) {
    case conf_lnum_utestloc:
       return conf_utestloc_local_get(fieldnum, instnum);
    case conf_lnum_globparam:
       return conf_globparam_local_get(fieldnum, instnum);
    case conf_lnum_train:
       return conf_train_local_get(fieldnum, instnum);
    case conf_lnum_topology:
       return conf_topology_local_get(fieldnum, instnum);
    case conf_lnum_boards:
       return conf_boards_local_get(fieldnum, instnum);
    }
    return 0;
}




void conf_local_set(unsigned int lconfnum, unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    switch (lconfnum) {
    case conf_lnum_utestloc:
       conf_utestloc_local_set(fieldnum, instnum, v);
        break;
    case conf_lnum_globparam:
       conf_globparam_local_set(fieldnum, instnum, v);
        break;
    case conf_lnum_train:
       conf_train_local_set(fieldnum, instnum, v);
        break;
    case conf_lnum_topology:
       conf_topology_local_set(fieldnum, instnum, v);
        break;
    case conf_lnum_boards:
       conf_boards_local_set(fieldnum, instnum, v);
        break;
    }
}


