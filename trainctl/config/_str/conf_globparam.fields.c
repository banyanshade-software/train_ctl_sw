// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_globparam.h"
#include "conf_globparam.propag.h"



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

int conf_globparam_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "pwmfreq")) {
         return conf_numfield_globparam_pwmfreq;
    } else if (!strcmp(str, "test_mode")) {
         return conf_numfield_globparam_test_mode;
    } else if (!strcmp(str, "oscilo")) {
         return conf_numfield_globparam_oscilo;
    }
    return -1;
}

const char *conf_globparam_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_globparam_pwmfreq) {
         return "pwmfreq";
    } else if (f == conf_numfield_globparam_test_mode) {
         return "test_mode";
    } else if (f == conf_numfield_globparam_oscilo) {
         return "oscilo";
    }
    return NULL;
}




// end
