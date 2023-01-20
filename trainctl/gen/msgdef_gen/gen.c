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
tyoedef enum {\n");

    int val = 0;
    int num = 0;
    for (msg_node_t *n = root; n; n=n->next) {
        switch (n->tag) {
        case MSG_NODE_START:
            val = n->value;
            if (num) printf("\n");
            printf("\t// ---------------------------- group 0x%2.2X\n", val);
            break;
        case MSG_NODE_MSG:
            printf("\t%-30s, // 0x%2.2x (%d) %s\n",
                n->string,
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

