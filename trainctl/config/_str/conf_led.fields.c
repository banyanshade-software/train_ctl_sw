// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_led.h"
#include "conf_led.propag.h"



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

int conf_led_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "port_led")) {
         return conf_numfield_port_led;
    } else if (!strcmp(str, "pin_led")) {
         return conf_numfield_pin_led;
    } else if (!strcmp(str, "defprog")) {
         return conf_numfield_defprog;
    }
    return -1;
}

const char *conf_led_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_port_led) {
         return "port_led";
    } else if (f == conf_numfield_pin_led) {
         return "pin_led";
    } else if (f == conf_numfield_defprog) {
         return "defprog";
    }
    return NULL;
}




// end
