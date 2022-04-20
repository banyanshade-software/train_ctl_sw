// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_train.h"
#include "conf_train.propag.h"



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

int conf_train_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "kP")) {
         return conf_numfield_kP;
    } else if (!strcmp(str, "kI")) {
         return conf_numfield_kI;
    } else if (!strcmp(str, "kD")) {
         return conf_numfield_kD;
    } else if (!strcmp(str, "dec")) {
         return conf_numfield_dec;
    } else if (!strcmp(str, "acc")) {
         return conf_numfield_acc;
    } else if (!strcmp(str, "volt_policy")) {
         return conf_numfield_volt_policy;
    } else if (!strcmp(str, "enabled")) {
         return conf_numfield_enabled;
    } else if (!strcmp(str, "enable_inertia")) {
         return conf_numfield_enable_inertia;
    } else if (!strcmp(str, "enable_pid")) {
         return conf_numfield_enable_pid;
    } else if (!strcmp(str, "reversed")) {
         return conf_numfield_reversed;
    } else if (!strcmp(str, "bemfIIR")) {
         return conf_numfield_bemfIIR;
    } else if (!strcmp(str, "postIIR")) {
         return conf_numfield_postIIR;
    } else if (!strcmp(str, "slipping")) {
         return conf_numfield_slipping;
    } else if (!strcmp(str, "pose_per_cm")) {
         return conf_numfield_pose_per_cm;
    } else if (!strcmp(str, "trainlen_left")) {
         return conf_numfield_trainlen_left;
    } else if (!strcmp(str, "trainlen_right")) {
         return conf_numfield_trainlen_right;
    }
    return -1;
}

const char *conf_train_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_kP) {
         return "kP";
    } else if (f == conf_numfield_kI) {
         return "kI";
    } else if (f == conf_numfield_kD) {
         return "kD";
    } else if (f == conf_numfield_dec) {
         return "dec";
    } else if (f == conf_numfield_acc) {
         return "acc";
    } else if (f == conf_numfield_volt_policy) {
         return "volt_policy";
    } else if (f == conf_numfield_enabled) {
         return "enabled";
    } else if (f == conf_numfield_enable_inertia) {
         return "enable_inertia";
    } else if (f == conf_numfield_enable_pid) {
         return "enable_pid";
    } else if (f == conf_numfield_reversed) {
         return "reversed";
    } else if (f == conf_numfield_bemfIIR) {
         return "bemfIIR";
    } else if (f == conf_numfield_postIIR) {
         return "postIIR";
    } else if (f == conf_numfield_slipping) {
         return "slipping";
    } else if (f == conf_numfield_pose_per_cm) {
         return "pose_per_cm";
    } else if (f == conf_numfield_trainlen_left) {
         return "trainlen_left";
    } else if (f == conf_numfield_trainlen_right) {
         return "trainlen_right";
    }
    return NULL;
}




// end
