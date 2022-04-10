
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "system.h"
#include "gen.h"


static void error(const char *s)
{
    fprintf(stderr, "error : %s\n", s);
    exit(1);
}

static const char *nstar(int n)
{
    if (n>5) n=5;
    n = 5-n;
    const char *st = "*****";
    return st+n;
}

static void gen_fields(config_node_t *node, FILE *output)
{
    for (; node; node = node->next) {
        if (node->tag == CONFIG_NODE_SUBREF) {
            char *s = node->string;
            fprintf(output, "    struct conf_%s %s\n", s, s);
            continue;
        }
        if (node->tag != CONFIG_NODE_FIELD) {
            error("bad tag expect field");
        }  
        fprintf(output, "    %s %s%s", node->type, nstar(node->nptr), node->string);
        if (node->bitfield) fprintf(output, ":%d", node->bitfield);
        if (node->array)    fprintf(output, "[%d]", node->array);
        fprintf(output, ";\n");
    }
}

void generate_hfile(config_node_t *root, int continue_next, FILE *output)
{
    fprintf(output, "// this file is generated automatically\n// do not modify\n");

    // generate sub sutruct
    for (config_node_t *node = root->subdefs; node; node = node->next) {
        char *n = node->string;
        fprintf(output, "struct conf_%s {\n", n);
        gen_fields(node->fields, output);
        fprintf(output, "};\n\n");

    }

    config_node_t *node = root->defs;
    for ( ; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag");
        }

        char *n = node->string;

        fprintf(output, "typedef struct conf_%s {\n", n);
        gen_fields(node->fields, output);
        fprintf(output, "} conf_%s_t;\n\n\n", n);
        fprintf(output, "int conf_%s_num_entries(void);\n", n);
        fprintf(output, "const conf_%s_t conf_%s_get(int num);\n\n", n, n);
        fprintf(output, "// handling config setup from master\n");
        fprintf(output, "void conf_%s_change(int instnum, int fieldnum, int16_t value);\n", n);
        if (!continue_next) break;
    }
}



static void gen_field_val(FILE *output, config_node_t *f, config_node_t *b, int numinst, config_node_t *tables, config_node_t *subs);
static config_node_t *find_board_value(config_node_t *values, int inst, const char *boardname);
static config_node_t *value_for_table(config_node_t *tables, char *tblname, char *colname, int numinst);
static config_node_t *find_by_name(config_node_t *node, const char *s, int *pc);

void generate_cfile(config_node_t *root, int continue_next, FILE *output, config_node_t *board)
{
    config_node_t *node = root->defs;
    config_node_t *tables = root->tables;
    fprintf(output, "// this file is generated automatically\n// do not modify\n");
    for ( ; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag, expect conf");
        }

        char *n = node->string;
        printf("// gen for %s\n\n", n);
        for (; board; board = board->next) {
            char *brduc = strdup(board->string);
            for (char *p = brduc; *p; p++) *p = toupper(*p);
            fprintf(output, "\n\n#ifdef TRN_BOARD_%s\n\n", brduc);

            config_node_t *b = find_board_value(node->numinst, -1, board->string);


            if (!b) error("no board or no default");
            fprintf(output, "int conf_%s_num_entries(void)\n{\n    return %s; // %d \n}\n\n", n, b->val->string, b->val->value);
            fprintf(output, "static conf_%s_t conf_%s[%s] = {\n", n,n, b->val->string);
            for (int numinst=0; numinst<b->val->value; numinst++) {
                fprintf(output, "  {     // %d\n", numinst);
                for (config_node_t *f = node->fields; f; f = f->next) {
                    gen_field_val(output, f, b, numinst, tables, root->subdefs);
                }
                fprintf(output, "  }%s\n", (numinst==b->val->value-1) ?  "" : ",");
            }
            fprintf(output, "};\n\n");
            fprintf(output, "#endif // TRN_BOARD_%s\n\n\n", brduc);
        }
    }
}

static int sameinstnum(int inst, int binst)
{
    if (inst == -1) return 1;
    if (binst  == -1) return 1;
    if (inst == binst) return 1;
    return 0;
}

static config_node_t *find_board_value(config_node_t *values, int inst, const char *boardname)
{
      config_node_t *bvdefault = NULL;
    config_node_t *bv = values;
    for (; bv;  bv = bv->next) {
        if (!strcmp(bv->string, "all") && sameinstnum(inst, bv->value)) {
            return bv;
        }
        if (!strcmp(bv->string, "default")) bvdefault = bv;
        if (!strcmp(bv->string, boardname) && sameinstnum(inst, bv->value)) return bv;
    }
    if (!bvdefault) error("no board or no default");
    return bvdefault;
}

static void gen_field_val(FILE *output, config_node_t *f, config_node_t *b, int numinst, config_node_t *tables, config_node_t *subs)
{
    if (f->tag == CONFIG_NODE_SUBREF) {
        //fprintf(output, "// subref %s\n", f->string);
        fprintf(output, "     .%s = {\n", f->string);
        config_node_t *sub = find_by_name(subs, f->string, NULL);
        if (!sub) {
            error("no such sub");
            exit(1);
        }
        for (config_node_t *sf = sub->fields; sf; sf = sf->next) {
            gen_field_val(output, sf, b, numinst, tables, subs);
        }
        
        fprintf(output, "     },\n");
        return;
    }
    config_node_t *v = find_board_value(f->boardvalues, numinst, b->string);
    config_node_t *val = v->val;
    if (val->tag == CONFIG_NODE_TABLEREF) {
        //printf("//table %s col %s inst %d\n", val->tablename, val->colname, numinst);
        config_node_t *v = value_for_table(tables, val->tablename, val->colname, numinst);
        if (v) val = v;
    }
   
    fprintf(output, "     .%s = %s\n", // \t\t// %s:%d,%s:%d tag:%d\n",
        f->string,
        val ? val->string : "missing");
        //b->string, numinst, v->string, v->value, val->tag);
}

// tables management

static config_node_t *find_by_name(config_node_t *node, const char *s, int *pc)
{
    int n=0;
    for ( ; node; node = node->next) {
        if (!strcmp(node->string, s)) {
            if (pc) *pc = n;
            return node;
        }
        n++;
    }
    if (pc) *pc = -1;
    return NULL;
}

static config_node_t *find_by_index(config_node_t *node, int idx)
{
    int n=0;
    for ( ; node; node = node->next) {
        if (n==idx) return node;
        n++;
    }
    return NULL;
}


static int table_find_colindex(config_node_t *coldef, char *string)
{
    int n;
    assert(coldef->tag == CONFIG_NODE_TABLELINE);
    find_by_name(coldef->lineval, string, &n);
    return n;
}

static config_node_t *value_for_table(config_node_t *tables, char *tblname, char *colname, int numinst)
{
    config_node_t *tbl = find_by_name(tables, tblname, NULL);
    if (!tbl) {
        fprintf(stderr, "no such table '%s'\n", tblname);
        return NULL;
    }
    assert(tbl->tag == CONFIG_NODE_TABLE);
    int cidx = table_find_colindex(tbl->coldef, colname);
    if (cidx<0) {
        fprintf(stderr, "no such column '%s' in %s\n", colname, tblname);
        return NULL;
    }
    config_node_t *line = find_by_index(tbl->lines, numinst);
    if (!line) {
        fprintf(stderr, "no such line %d in %s\n", numinst, tblname);
        return NULL;
    }
    assert(line->tag == CONFIG_NODE_TABLELINE);
    config_node_t *v = find_by_index(line->lineval, cidx);
    if (!v) {
        fprintf(stderr, "no such column %d in line %d of %s\n", cidx, numinst, tblname);
        return NULL;
    }
    return v;
}

    
    
