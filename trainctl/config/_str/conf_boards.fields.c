// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "../conf_boards.h"
#include "../conf_boards.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"

int conf_boards_fieldnum(const char *str)
{
    if (0) {
    } else if (!strcmp(str, "uuid")) {
         return conf_numfield_boards_uuid;
    } else if (!strcmp(str, "board_type")) {
         return conf_numfield_boards_board_type;
    } else if (!strcmp(str, "disable")) {
         return conf_numfield_boards_disable;
    }
    return -1;
}

const char *conf_boards_fieldname(int f)
{
    if (0) {
    } else if (f == conf_numfield_boards_uuid) {
         return "uuid";
    } else if (f == conf_numfield_boards_board_type) {
         return "board_type";
    } else if (f == conf_numfield_boards_disable) {
         return "disable";
    }
    return NULL;
}




// end
