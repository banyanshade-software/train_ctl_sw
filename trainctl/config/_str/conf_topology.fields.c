// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_topology.h"
#include "../conf_topology.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_topology_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "canton_addr")) {
         return conf_numfield_topology_canton_addr;
    } else if (!strcmp(str, "ina_segnum")) {
         return conf_numfield_topology_ina_segnum;
    } else if (!strcmp(str, "steep")) {
         return conf_numfield_topology_steep;
    } else if (!strcmp(str, "length_cm")) {
         return conf_numfield_topology_length_cm;
    } else if (!strcmp(str, "left1")) {
         return conf_numfield_topology_left1;
    } else if (!strcmp(str, "left2")) {
         return conf_numfield_topology_left2;
    } else if (!strcmp(str, "ltn")) {
         return conf_numfield_topology_ltn;
    } else if (!strcmp(str, "right1")) {
         return conf_numfield_topology_right1;
    } else if (!strcmp(str, "right2")) {
         return conf_numfield_topology_right2;
    } else if (!strcmp(str, "rtn")) {
         return conf_numfield_topology_rtn;
    } else if (!strcmp(str, "p0")) {
         return conf_numfield_topology_p0;
    } else if (!strcmp(str, "points")) {
         return conf_numfield_topology_points;
    }
    return -1;
}

const char *conf_topology_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_topology_canton_addr) {
         return "canton_addr";
    } else if (f == conf_numfield_topology_ina_segnum) {
         return "ina_segnum";
    } else if (f == conf_numfield_topology_steep) {
         return "steep";
    } else if (f == conf_numfield_topology_length_cm) {
         return "length_cm";
    } else if (f == conf_numfield_topology_left1) {
         return "left1";
    } else if (f == conf_numfield_topology_left2) {
         return "left2";
    } else if (f == conf_numfield_topology_ltn) {
         return "ltn";
    } else if (f == conf_numfield_topology_right1) {
         return "right1";
    } else if (f == conf_numfield_topology_right2) {
         return "right2";
    } else if (f == conf_numfield_topology_rtn) {
         return "rtn";
    } else if (f == conf_numfield_topology_p0) {
         return "p0";
    } else if (f == conf_numfield_topology_points) {
         return "points";
    }
    return NULL;
}




// end
