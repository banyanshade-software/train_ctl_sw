
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "system.h"



static config_node_t *_find_by_name_and_type(config_node_t *node, const char *s, node_type_t t, int *pn)
{
    int n = 0;
    for ( ; node; node = node->next) {
        n++;
        if (s && strcmp(node->string, s)) continue;
        if ((t != -1) && (node->tag != t)) continue;
        if (pn) *pn = n-1;
        return node;
    }
    return NULL;
}

config_node_t *find_by_name(config_node_t *node, const char *s, int *pc)
{
    return _find_by_name_and_type(node, s, -1, pc);
}


config_node_t *find_by_name_and_type(config_node_t *node, const char *s, node_type_t t)
{
    return _find_by_name_and_type(node, s, t, NULL);
}



config_node_t *find_by_index(config_node_t *node, int idx)
{
    int n=0;
    config_node_t *last;
    for ( ; node; node = node->next) {
        if (n==idx) return node;
        last = node;
        n++;
    }
    if (-1 == idx) return last;
    return NULL;
}

void apply_field(config_node_t *root, config_node_t *fields, int recsubref, void (*func)(config_node_t *n, FILE *output, int num), FILE *output, int startnum)
{
    int num = startnum;
    for (config_node_t *f = fields; f; f=f->next) {
        if (f->tag == CONFIG_NODE_SUBREF) {
            if (!recsubref) continue;
            if (-1 == recsubref) {
                func(f, output, num);
                num++;
                continue;
            }
            config_node_t *sub = find_by_name(root->subdefs, f->string, NULL);
            if (!sub) {
                fprintf(stderr, "no such sub '%s'\n", f->string);
                exit(1);
            }
            assert(CONFIG_NODE_SUBCONF == sub->tag);
            apply_field(root, sub->fields, recsubref, func, output, num);
        } else {
            assert(f->tag == CONFIG_NODE_FIELD);
            if (-1 == recsubref) continue;
            func(f, output, num);
            num++;
        }
    }
}

