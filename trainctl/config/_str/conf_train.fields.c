// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_train.h"
#include "../conf_train.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_train_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "locotype")) {
         return conf_numfield_train_locotype;
    } else if (!strcmp(str, "enabled")) {
         return conf_numfield_train_enabled;
    } else if (!strcmp(str, "trainlen_left_cm")) {
         return conf_numfield_train_trainlen_left_cm;
    } else if (!strcmp(str, "trainlen_right_cm")) {
         return conf_numfield_train_trainlen_right_cm;
    }
    return -1;
}

const char *conf_train_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_train_locotype) {
         return "locotype";
    } else if (f == conf_numfield_train_enabled) {
         return "enabled";
    } else if (f == conf_numfield_train_trainlen_left_cm) {
         return "trainlen_left_cm";
    } else if (f == conf_numfield_train_trainlen_right_cm) {
         return "trainlen_right_cm";
    }
    return NULL;
}




// end
