// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_canton.h"
#include "conf_canton.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_canton_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "reverse_bemf")) {
         return conf_numfield_canton_reverse_bemf;
    }
    return -1;
}

const char *conf_canton_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_canton_reverse_bemf) {
         return "reverse_bemf";
    }
    return NULL;
}




// end
