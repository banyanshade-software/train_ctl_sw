// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_servo.h"
#include "../conf_servo.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_servo_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "direction")) {
         return conf_numfield_servo_direction;
    } else if (!strcmp(str, "min")) {
         return conf_numfield_servo_min;
    } else if (!strcmp(str, "max")) {
         return conf_numfield_servo_max;
    } else if (!strcmp(str, "spd")) {
         return conf_numfield_servo_spd;
    } else if (!strcmp(str, "pos0")) {
         return conf_numfield_servo_pos0;
    }
    return -1;
}

const char *conf_servo_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_servo_direction) {
         return "direction";
    } else if (f == conf_numfield_servo_min) {
         return "min";
    } else if (f == conf_numfield_servo_max) {
         return "max";
    } else if (f == conf_numfield_servo_spd) {
         return "spd";
    } else if (f == conf_numfield_servo_pos0) {
         return "pos0";
    }
    return NULL;
}




// end
