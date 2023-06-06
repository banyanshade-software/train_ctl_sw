#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "system.h"
#include "gen.h"


void generate_msgdef_header(msg_node_t *root)
{
    printf("\
// this file is generated automatically, do not edit\n\
#ifndef trainmsgdef_h\n\
#define trainmsgdef_h\n\
\n\
typedef enum {\n");

    int val = 0;
    int num = 0;
    int ng = 0;
    for (msg_node_t *n = root; n; n=n->next) {
        switch (n->tag) {
        case MSG_NODE_START:
            val = n->value;
            if (num) printf("\n");
            printf("\t// ---------------------------- group 0x%2.2X\n", val);
            ng = 1;
            break;
        case MSG_NODE_MSG:
            if (ng) {
                printf("\t%-23s = 0x%2.2X, ", n->string, val);
                ng = 0;
            } else {
                printf("\t%-30s, ", n->string);
            }
            printf("// 0x%2.2x (%d) %s\n",
                val, val,
                n->comment ? n->comment : "");
            val++;
            break;
        default:
            printf("// XXX %d\n", n->tag);
            break;
        }
        num++;
    }

    printf("\
} cmd_msg_t;\n\n\
#endif\n");

}

static const char * typestr(msg_type_tag_t s)
{
    switch (s) {
    default:
    case TYPE_V32: return "TYPE_V32";
    case TYPE_B4:  return "TYPE_B4";
    case TYPE_V40: return "TYPE_V40";
    case TYPE_VCU: return "TYPE_VCU";
    case TYPE_V16: return "TYPE_V16";
    }
}

void generate_msgdef_str(msg_node_t *root)
{
    printf("\
// this file is generated automatically, do not edit\n\
#include <stdint.h>\n\
#include \"trainmsgdef.h\"\n\
#include \"trainmsgstr.h\"\n\
\n\
const char *traincmd_name(uint8_t cmd)\n{\n\
    switch(cmd) {\n\
    default : return \"???\";\n");

    for (msg_node_t *n = root; n; n=n->next) {
        switch (n->tag) {
        default: continue;
        case MSG_NODE_MSG:
                printf("\tcase %-20s: return \"%s\";\n", n->string, n->string);
                break;
        } 
    }

    printf("\
    }\n\
}\n\n");

    printf("\
msg_type_t traincmd_format(uint8_t cmd)\n{\n\
    switch(cmd) {\n\
    default : return CMD_TYPE_V32;\n");

    for (msg_node_t *n = root; n; n=n->next) {
        switch (n->tag) {
        default: continue;
        case MSG_NODE_MSG:
                printf("\tcase %-20s: return CMD_%s;\n", n->string, typestr(n->typef));
                break;
        } 
    }
    printf("\
    }\n\
}\n\n");

}

