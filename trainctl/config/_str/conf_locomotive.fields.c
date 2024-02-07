// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_locomotive.h"
#include "../conf_locomotive.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_locomotive_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "kP")) {
         return conf_numfield_pidctl_kP;
    } else if (!strcmp(str, "kI")) {
         return conf_numfield_pidctl_kI;
    } else if (!strcmp(str, "kD")) {
         return conf_numfield_pidctl_kD;
    } else if (!strcmp(str, "dec")) {
         return conf_numfield_inertia_dec;
    } else if (!strcmp(str, "acc")) {
         return conf_numfield_inertia_acc;
    } else if (!strcmp(str, "volt_policy")) {
         return conf_numfield_locomotive_volt_policy;
    } else if (!strcmp(str, "enable_inertia")) {
         return conf_numfield_locomotive_enable_inertia;
    } else if (!strcmp(str, "enable_pid")) {
         return conf_numfield_locomotive_enable_pid;
    } else if (!strcmp(str, "reversed")) {
         return conf_numfield_locomotive_reversed;
    } else if (!strcmp(str, "min_power_acc")) {
         return conf_numfield_locomotive_min_power_acc;
    } else if (!strcmp(str, "min_power_dec")) {
         return conf_numfield_locomotive_min_power_dec;
    } else if (!strcmp(str, "bemfIIR")) {
         return conf_numfield_locomotive_bemfIIR;
    } else if (!strcmp(str, "postIIR")) {
         return conf_numfield_locomotive_postIIR;
    } else if (!strcmp(str, "slipping")) {
         return conf_numfield_locomotive_slipping;
    } else if (!strcmp(str, "pose_per_cm")) {
         return conf_numfield_locomotive_pose_per_cm;
    }
    return -1;
}

const char *conf_locomotive_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_pidctl_kP) {
         return "kP";
    } else if (f == conf_numfield_pidctl_kI) {
         return "kI";
    } else if (f == conf_numfield_pidctl_kD) {
         return "kD";
    } else if (f == conf_numfield_inertia_dec) {
         return "dec";
    } else if (f == conf_numfield_inertia_acc) {
         return "acc";
    } else if (f == conf_numfield_locomotive_volt_policy) {
         return "volt_policy";
    } else if (f == conf_numfield_locomotive_enable_inertia) {
         return "enable_inertia";
    } else if (f == conf_numfield_locomotive_enable_pid) {
         return "enable_pid";
    } else if (f == conf_numfield_locomotive_reversed) {
         return "reversed";
    } else if (f == conf_numfield_locomotive_min_power_acc) {
         return "min_power_acc";
    } else if (f == conf_numfield_locomotive_min_power_dec) {
         return "min_power_dec";
    } else if (f == conf_numfield_locomotive_bemfIIR) {
         return "bemfIIR";
    } else if (f == conf_numfield_locomotive_postIIR) {
         return "postIIR";
    } else if (f == conf_numfield_locomotive_slipping) {
         return "slipping";
    } else if (f == conf_numfield_locomotive_pose_per_cm) {
         return "pose_per_cm";
    }
    return NULL;
}




// end
