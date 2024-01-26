// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_globparam.h"
#include "../conf_globparam.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_globparam_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "pwmfreq")) {
         return conf_numfield_globparam_pwmfreq;
    } else if (!strcmp(str, "test_mode")) {
         return conf_numfield_globparam_test_mode;
    } else if (!strcmp(str, "oscillo")) {
         return conf_numfield_globparam_oscillo;
    } else if (!strcmp(str, "ignoreIna3221")) {
         return conf_numfield_globparam_ignoreIna3221;
    } else if (!strcmp(str, "enable_adjust")) {
         return conf_numfield_globparam_enable_adjust;
    } else if (!strcmp(str, "enable_adjust_steep")) {
         return conf_numfield_globparam_enable_adjust_steep;
    } else if (!strcmp(str, "disable_pose_update")) {
         return conf_numfield_globparam_disable_pose_update;
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
    } else if (f == conf_numfield_globparam_oscillo) {
         return "oscillo";
    } else if (f == conf_numfield_globparam_ignoreIna3221) {
         return "ignoreIna3221";
    } else if (f == conf_numfield_globparam_enable_adjust) {
         return "enable_adjust";
    } else if (f == conf_numfield_globparam_enable_adjust_steep) {
         return "enable_adjust_steep";
    } else if (f == conf_numfield_globparam_disable_pose_update) {
         return "disable_pose_update";
    }
    return NULL;
}




// end
