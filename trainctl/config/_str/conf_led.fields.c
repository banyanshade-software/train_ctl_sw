// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_led.h"
#include "../conf_led.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_led_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "defprog")) {
         return conf_numfield_led_defprog;
    }
    return -1;
}

const char *conf_led_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_led_defprog) {
         return "defprog";
    }
    return NULL;
}




// end
