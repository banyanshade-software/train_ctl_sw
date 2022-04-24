// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_turnout.h"
#include "../conf_turnout.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_turnout_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "reverse")) {
         return conf_numfield_turnout_reverse;
    }
    return -1;
}

const char *conf_turnout_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_turnout_reverse) {
         return "reverse";
    }
    return NULL;
}




// end
