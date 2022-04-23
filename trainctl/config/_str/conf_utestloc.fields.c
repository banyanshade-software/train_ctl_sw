// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_utestloc.h"
#include "conf_utestloc.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif

int conf_utestloc_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "alpha")) {
         return conf_numfield_utestloc_alpha;
    } else if (!strcmp(str, "beta")) {
         return conf_numfield_utestloc_beta;
    }
    return -1;
}

const char *conf_utestloc_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_utestloc_alpha) {
         return "alpha";
    } else if (f == conf_numfield_utestloc_beta) {
         return "beta";
    }
    return NULL;
}




// end
