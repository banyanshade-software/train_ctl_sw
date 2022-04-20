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

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif

int conf_canton_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "volts_cv")) {
         return conf_numfield_volts_cv;
    } else if (!strcmp(str, "volt_port_b0")) {
         return conf_numfield_volt_port_b0;
    } else if (!strcmp(str, "volt_b0")) {
         return conf_numfield_volt_b0;
    } else if (!strcmp(str, "volt_port_b1")) {
         return conf_numfield_volt_port_b1;
    } else if (!strcmp(str, "volt_b1")) {
         return conf_numfield_volt_b1;
    } else if (!strcmp(str, "volt_port_b2")) {
         return conf_numfield_volt_port_b2;
    } else if (!strcmp(str, "volt_b2")) {
         return conf_numfield_volt_b2;
    } else if (!strcmp(str, "pwm_timer_num")) {
         return conf_numfield_pwm_timer_num;
    } else if (!strcmp(str, "ch0")) {
         return conf_numfield_ch0;
    } else if (!strcmp(str, "ch1")) {
         return conf_numfield_ch1;
    } else if (!strcmp(str, "notif_bemf")) {
         return conf_numfield_notif_bemf;
    } else if (!strcmp(str, "reverse_bemf")) {
         return conf_numfield_reverse_bemf;
    }
    return -1;
}

const char *conf_canton_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_volts_cv) {
         return "volts_cv";
    } else if (f == conf_numfield_volt_port_b0) {
         return "volt_port_b0";
    } else if (f == conf_numfield_volt_b0) {
         return "volt_b0";
    } else if (f == conf_numfield_volt_port_b1) {
         return "volt_port_b1";
    } else if (f == conf_numfield_volt_b1) {
         return "volt_b1";
    } else if (f == conf_numfield_volt_port_b2) {
         return "volt_port_b2";
    } else if (f == conf_numfield_volt_b2) {
         return "volt_b2";
    } else if (f == conf_numfield_pwm_timer_num) {
         return "pwm_timer_num";
    } else if (f == conf_numfield_ch0) {
         return "ch0";
    } else if (f == conf_numfield_ch1) {
         return "ch1";
    } else if (f == conf_numfield_notif_bemf) {
         return "notif_bemf";
    } else if (f == conf_numfield_reverse_bemf) {
         return "reverse_bemf";
    }
    return NULL;
}




// end
