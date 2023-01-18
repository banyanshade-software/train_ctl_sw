
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "system.h"
#include "gen.h"
#include "nodeutil.h"


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



// ------------------------------------------------------------

static void filename_for(char *buf, config_node_t *confnode, char *ext, char *prefix)
{
    sprintf(buf, "%sconf_%s.%s", prefix, confnode->string, ext);
}

static void include_code(FILE *output, config_node_t *attr, int v)
{
    for ( ; attr; attr = attr->next) {
        if (attr->tag != CONFIG_NODE_ATTR_CODE) continue;
        if (attr->value != v) continue;
        fputs(attr->string, output);
    }
}

static FILE *genfile_h_create(config_node_t *confnode, config_node_t *root, int propag)
{
    assert((confnode->tag == CONFIG_NODE_CONF) || (confnode->tag == CONFIG_NODE_SUBCONF));
    char n[128];
    filename_for(n, confnode, propag ?  "propag.h" : "h", "");
    FILE *F = fopen(n, "w");
    assert(F);
    char *p = propag ? "propag_":"";
    char *s = confnode->string;
    fprintf(F, "// this file is generated automatically by config\n// DO NOT EDIT\n\n\n");
    fprintf(F, "#ifndef _conf_%s_%sH_\n#define _conf_%s_%sH_\n\n#include <stdint.h>\n", 
        s, p, s, p);
    include_code(F, root->globattrib, propag ? 3 : 1);
    include_code(F, confnode->attr, propag ? 3 : 1);
    return F;
}

static void genfile_h_close(FILE *F)
{
    fprintf(F, "\n\n\n#endif\n");
    fclose(F);
}


static FILE *genfile_c_create(config_node_t *confnode, config_node_t *root, int fn)
{
    assert((confnode->tag == CONFIG_NODE_CONF) || (confnode->tag == CONFIG_NODE_SUBCONF));
    char n[128];
    filename_for(n, confnode, fn ? "fields.c" : "c", fn ? "_str/" : "");
    FILE *F = fopen(n, "w");
    assert(F);
    char *path = "";
    if (fn) path="../";
    fprintf(F, "// this file is generated automatically by config\n// DO NOT EDIT\n\n\n");
    fprintf(F, "#include <stdint.h>\n#include <stddef.h>\n");
    if (fn) fprintf(F, "#include <string.h>\n");
    fprintf(F, "#include \"%sconf_%s.h\"\n", path, confnode->string);
    if (confnode->tag == CONFIG_NODE_CONF) {
        fprintf(F, "#include \"%sconf_%s.propag.h\"\n", path, confnode->string);
    }
    fprintf(F, "\n\n");
    include_code(F, root->globattrib, 2);
    if (!fn) include_code(F, confnode->attr, 2);
    return F;
}

static void genfile_c_close(FILE *F)
{
    fprintf(F, "\n\n\n// end\n");
    fclose(F);
}


// ------------------------------------------------------------
static void gen_field_val(FILE *output, config_node_t *f, config_node_t *b, int numinst, config_node_t *tables, config_node_t *subs);
static config_node_t *find_board_value(config_node_t *values, int inst, const char *boardname);
static config_node_t *value_for_table(config_node_t *tables, char *tblname, char *colname, int numinst);
// ------------------------------------------------------------


static int _gen_subref(config_node_t *node, FILE *output, int num)
{
    char *s = node->string;
    fprintf(output, "    struct conf_%s %s;\n", s, s);
    return 1;
}

static int _gen_field(config_node_t *node, FILE *output, int num)
{
    if (node->tag != CONFIG_NODE_FIELD) {
        error("bad tag expect field");
    }  
    fprintf(output, "    %s %s%s", node->type, nstar(node->nptr), node->string);
    if (node->bitfield) fprintf(output, ":%d", node->bitfield);
    if (node->array)    fprintf(output, "[%d]", node->array);
    fprintf(output, ";\n");
    return 1;
}

static void gen_fields(config_node_t *node, FILE *output)
{
    apply_field(NULL, node, -1, _gen_subref, output, 0);
    apply_field(NULL, node,  0, _gen_field,  output, 0);
}

static int _gen_incsub(config_node_t *node, FILE *output, int num)
{
    char n[128];
    filename_for(n, node, "h", "");
    fprintf(output, "#include \"%s\"\n", n);
    return 1;
}
static void generate_incsub(config_node_t *field, FILE *output)
{
    apply_field(NULL, field,  -1, _gen_incsub,  output, 0);
    fprintf(output, "\n\n");
}

static void get_storetype(config_node_t *node, int *ptype, int *pnum, int *gentpl)
{
    config_node_t *nt = find_by_name_and_type(node->attr, NULL, CONFIG_NODE_ATTR_STORETYPE);
    if (!nt) {
        fprintf(stderr, "no config store type for %s\n", node->string);
        exit(1);
        *ptype = 0;
    } else {
        *ptype = nt->value;
    }

    config_node_t *nn = find_by_name_and_type(node->attr, NULL, CONFIG_NODE_ATTR_STORENUM);
    if (!nn) {
        fprintf(stderr, "no config store num for %s\n", node->string);
        exit(1);
    } else {
        *pnum = nn->value;
    }

    config_node_t *nk = find_by_name_and_type(node->attr, NULL, CONFIG_NODE_ATTR_GENTPL);
    if (nk) {
        *gentpl = 1;
    } else {
        *gentpl = 0;
    }
}

static void generate_hfile_normal(config_node_t *root, config_node_t *boards)
{

    // generate sub struct
    for (config_node_t *node = root->subdefs; node; node = node->next) {
        FILE *output = genfile_h_create(node, root, 0);
        char *n = node->string;
        generate_incsub(node->fields, output);
        fprintf(output, "struct conf_%s {\n", n);
        gen_fields(node->fields, output);
        fprintf(output, "};\n\n");
        genfile_h_close(output);
    }

    config_node_t *node = root->defs;
    for ( ; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag");
        }

        FILE *output = genfile_h_create(node, root, 0);

        generate_incsub(node->fields, output);
        char *n = node->string;

        fprintf(output, "typedef struct conf_%s {\n", n);
        gen_fields(node->fields, output);
        fprintf(output, "} conf_%s_t;\n\n\n", n);
        fprintf(output, "unsigned int conf_%s_num_entries(void);\n", n);
        fprintf(output, "const conf_%s_t *conf_%s_get(int num);\n\n", n, n);
        // get max entries
        char *nuc = strdup(n);
        for (char *p = nuc; *p; p++) *p = toupper(*p);

        int max=0;
        for (config_node_t *board = boards; board; board = board->next) {
            char *brduc = strdup(board->string);
            for (char *p = brduc; *p; p++) *p = toupper(*p);
            fprintf(output, "\n\n#ifdef TRN_BOARD_%s\n", brduc);
            config_node_t *b = find_board_value(node->numinst, -1, board->string);
            fprintf(output, "#define NUM_%sS %s // %d \n", nuc, b->val->string, b->val->value);
            fprintf(output, "#endif // TRN_BOARD_%s\n\n", brduc);
            if (b->val->value>max) max = b->val->value;
        }
        fprintf(output, "\n#define MAX_%sS %d\n\n", nuc, max);

        int storetype, storenum, gentpl;
        get_storetype(node, &storetype, &storenum, &gentpl);
        if (gentpl) {
            fprintf(output, "\nconst conf_%s_t *conf_%s_template(void);\n\n", n,n);
        }

        //fprintf(output, "// handling config setup from master\n");
        //fprintf(output, "void conf_%s_change(int instnum, int fieldnum, int16_t value);\n", n);
        genfile_h_close(output);
    }
}

static int _gen_propagdef(config_node_t *f, FILE *output, int num)
{
    if (!f->configurable) return 0;
    fprintf(output, "#define conf_numfield_%s_%s \t\t%d\n", f->parentconf->string, f->string, num);
    return 1;
}
static void generate_hfile_propag(config_node_t *root)
{
    for (config_node_t *node = root->defs; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag");
        }

        FILE *output = genfile_h_create(node, root, 1);
        char *n = node->string;
        fprintf(output, "// %s for propag\n\n", n);
        fprintf(output, "\nint conf_%s_propagate(unsigned int numinst, unsigned int numfield, int32_t value);\n\n", n);
        fprintf(output, "\nint32_t conf_%s_default_value(unsigned int numinst, unsigned int numfield, unsigned int board);\n\n", n);


    
        int storetype, storenum, gentpl;
        get_storetype(node, &storetype, &storenum, &gentpl);
        char t = storetype ? 'l' : 'p';
        fprintf(output, "#define conf_%cnum_%s %d\n\n", t, node->string, storenum);

        apply_field(root, node->fields, 1, _gen_propagdef, output, 0);

        if (storetype==1) {
            fprintf(output, "\n\nvoid *conf_%s_ptr(void);\n", n);
            fprintf(output, "int32_t conf_%s_local_get(unsigned int fieldnum, unsigned int instnum);\n", n);
            fprintf(output, "void conf_%s_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v);\n", n);
        }

        genfile_h_close(output);
    }
}

static void apply_conf(config_node_t *root, int (*filter)(config_node_t *ncf), void (*func)(const char *n, FILE *output), FILE *output)
{
    for (config_node_t *node = root->defs; node; node = node->next) {
        if (filter && !filter(node)) continue;
        char *n = node->string;
        func(n, output);
    }
}

int _genf_type0(config_node_t *node)
{
    int storetype, storenum, gentpl;
    get_storetype(node, &storetype, &storenum, &gentpl);
    if (storetype == 0) return 1;
    return 0;
}
int _genf_type1(config_node_t *node)
{
    int storetype, storenum, gentpl;
    get_storetype(node, &storetype, &storenum, &gentpl);
    if (storetype == 1) return 1;
    return 0;
}
void _gen_propag(const char *n, FILE *output)
{
    fprintf(output, "    case conf_pnum_%s:\n", n);
    fprintf(output, "        conf_%s_propagate(instnum, fieldnum, v);\n", n);
    fprintf(output, "        break;\n");
}

void _gen_inc(const char *n, FILE *output)
{
     fprintf(output, "#include \"conf_%s.h\"\n", n);
     fprintf(output, "#include \"conf_%s.propag.h\"\n", n);
}

void _gen_defval(const char *n, FILE *output)
{
      fprintf(output, "    case conf_pnum_%s:\n", n);
      fprintf(output, "        return conf_%s_default_value(instnum, fieldnum, board);\n", n);
      fprintf(output, "        break;\n");
}

void _gen_ptr(const char *n, FILE *output)
{
      fprintf(output, "    case conf_lnum_%s:\n", n);
      fprintf(output, "       return conf_%s_ptr();\n", n);
}

void _gen_size(const char *n, FILE *output)
{
      fprintf(output, "    case conf_lnum_%s:\n", n);
      fprintf(output, "       return sizeof(conf_%s_t)*conf_%s_num_entries();\n", n, n);
}

void _gen_clget(const char *n, FILE *output)
{
       fprintf(output, "    case conf_lnum_%s:\n", n);
       fprintf(output, "       return conf_%s_local_get(fieldnum, instnum);\n", n);
}
void _gen_clset(const char *n, FILE *output)
{
       fprintf(output, "    case conf_lnum_%s:\n", n);
       fprintf(output, "       conf_%s_local_set(fieldnum, instnum, v);\n        break;\n", n);
}

void generate_cfile_global_propag(config_node_t *root)
{
    FILE *output = fopen("conf_gpropag.c", "w");
    if (!output) error("cant create propag.c");

    fprintf(output, "// this file is automatically generated\n");
    fprintf(output, "#include <stdint.h>\n#include <stddef.h>\n\n#include \"propag.h\"\n\n");
    apply_conf(root, NULL, _gen_inc, output);

    fprintf(output, "\n\nvoid conf_propagate(unsigned int confnum, unsigned int fieldnum, unsigned int instnum, int32_t v)\n");
    fprintf(output, "{\n    switch (confnum) {\n");
    apply_conf(root, _genf_type0, _gen_propag, output);
    fprintf(output, "    default: break;\n    }\n\n}\n\n");

    fprintf(output, "\n\nint32_t conf_default_value(unsigned int confnum, unsigned int fieldnum, unsigned int board, unsigned int instnum)\n");
    fprintf(output, "{\n    switch (confnum) {\n");
    apply_conf(root, _genf_type0, _gen_defval, output);
    fprintf(output, "    default: return 0;\n    break;\n    }\n\n}\n\n");

    fprintf(output, "\n\nvoid *conf_local_ptr(unsigned int lconfnum)\n");
    fprintf(output, "{\n    switch (lconfnum) {\n");
    fprintf(output, "       default: return NULL;\n");
    apply_conf(root, _genf_type1, _gen_ptr, output);
    fprintf(output, "    }\n    return NULL;\n}\n\n\n");

    fprintf(output, "\n\nunsigned int conf_local_size(unsigned int lconfnum)\n");
    fprintf(output, "{\n    switch (lconfnum) {\n");
    fprintf(output, "       default: return 0;\n");
    apply_conf(root, _genf_type1, _gen_size, output);
    fprintf(output, "    }\n    return 0;\n}\n\n\n");

    fprintf(output, "\n\nint32_t conf_local_get(unsigned int lconfnum, unsigned int fieldnum, unsigned int instnum)\n");
    fprintf(output, "{\n    switch (lconfnum) {\n");
    fprintf(output, "       default: return 0;\n");
    apply_conf(root, _genf_type1, _gen_clget, output);
    fprintf(output, "    }\n    return 0;\n}\n\n\n");

    fprintf(output, "\n\nvoid conf_local_set(unsigned int lconfnum, unsigned int fieldnum, unsigned int instnum, int32_t v)\n");
    fprintf(output, "{\n    switch (lconfnum) {\n");
    fprintf(output, "       default: break;\n");
    apply_conf(root, _genf_type1, _gen_clset, output);
    fprintf(output, "    }\n}\n\n\n");

    fclose(output);
}

void generate_hfiles(config_node_t *root, config_node_t *boards)
{
    generate_hfile_normal(root, boards);
    generate_hfile_propag(root);
}



static void gen_field_propag(FILE * output, config_node_t *fields, config_node_t *root);
static int _gen_fdefault(config_node_t *f, FILE *output, int num);
static config_node_t *_fdef_tables = NULL;
static int _gen_lset(config_node_t *f, FILE *output, int num);
static int _gen_lget(config_node_t *f, FILE *output, int num);

void generate_cfile(config_node_t *root, int continue_next, config_node_t *boards)
{
    config_node_t *node = root->defs;
    config_node_t *tables = root->tables;
    for ( ; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag, expect conf");
        }

        char *n = node->string;
        printf("--> config def %s\n", n);
        FILE *output = genfile_c_create(node, root, 0);


        for (config_node_t *board = boards; board; board = board->next) {
            printf("  --> gen for board %s\n", board->string);
            char *brduc = strdup(board->string);
            for (char *p = brduc; *p; p++) *p = toupper(*p);
            fprintf(output, "\n\n#ifdef TRN_BOARD_%s\n\n", brduc);

            config_node_t *b = find_board_value(node->numinst, -1, board->string);


            if (!b) error("no board or no default");
            fprintf(output, "unsigned int conf_%s_num_entries(void)\n{\n    return %s; // %d \n}\n\n", n, b->val->string, b->val->value);
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
        fprintf(output, "\n\nconst conf_%s_t *conf_%s_get(int num)\n", n, n);
        fprintf(output, "{\n  if (num<0) return NULL;\n    if ((unsigned int)num>=conf_%s_num_entries()) {\n        return NULL;\n    }\n", n);
        fprintf(output, "    return &conf_%s[num];\n}\n\n", n);

        int storetype, storenum, gentpl;
        get_storetype(node, &storetype, &storenum, &gentpl);

        if (storetype == 1) {
            fprintf(output, "\n\nvoid *conf_%s_ptr(void)\n",n);
            fprintf(output, "{\n    return &conf_%s[0];\n}\n\n", n);

            fprintf(output, "\n\nint32_t conf_%s_local_get(unsigned int fieldnum, unsigned int instnum)\n", n);
            fprintf(output, "{\n    const conf_%s_t *c = conf_%s_get(instnum);\n    if (!c) return 0;\n", n, n);
            fprintf(output, "    switch (fieldnum) {\n");
            fprintf(output, "    default: break;\n");
            apply_field(root, node->fields, 1, _gen_lget, output, 0);
            fprintf(output, "    }\n    return 0;\n}\n\n");

            fprintf(output, "\n\nvoid conf_%s_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)\n", n);
            fprintf(output, "{\n    conf_%s_t *ca = (conf_%s_t *) conf_%s_ptr();\n    if (!ca) return;\n", n, n, n);
            fprintf(output, "    conf_%s_t *c = &ca[instnum];\n", n);
            fprintf(output, "    switch (fieldnum) {\n");
            fprintf(output, "    default: break;\n");
            apply_field(root, node->fields, 1, _gen_lset, output, 0);
            fprintf(output, "    }\n\n}\n\n");
        }

        if (gentpl) {
            fprintf(output, "static const conf_%s_t %s_template = {\n", n, n);
            for (config_node_t *f = node->fields; f; f = f->next) {
                gen_field_val(output, f, NULL, 0, tables, root->subdefs);
            }
            fprintf(output, "  };\n");

            fprintf(output, "const conf_%s_t *conf_%s_template(void)\n{\n", n,n);
            fprintf(output, "  return &%s_template;\n", n);
            fprintf(output, "}\n\n");
        }

        fprintf(output, "// %s config store type %d num %d\n", node->string, storetype, storenum);

        if (storetype == 0) {
            fprintf(output, "int conf_%s_propagate(unsigned int numinst, unsigned int numfield, int32_t value)\n", n);
            fprintf(output, "{\n    if (numinst>=conf_%s_num_entries()) return -1;\n", n);
            fprintf(output, "    conf_%s_t *conf = &conf_%s[numinst];\n", n, n);
            fprintf(output, "    switch (numfield) {\n");
            fprintf(output, "    default: return -1;\n");
            gen_field_propag(output, node->fields, root);
            fprintf(output, "    }\n");
            fprintf(output, "    return 0;\n}\n\n\n");

            _fdef_tables = root->tables;
            fprintf(output, "int32_t conf_%s_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)\n", n);
            fprintf(output, "{\n    (void) boardnum;\n");
            fprintf(output, "    (void) numinst;\n");
            fprintf(output, "    //if (numinst>=conf_%s_num_entries()) return 0;\n", n);
            fprintf(output, "    switch (numfield) {\n");
            fprintf(output, "    default: return 0;\n");
            apply_field(root, node->fields, 1, _gen_fdefault, output, 0);
            fprintf(output, "    }\n");
            fprintf(output, "    return 0;\n}\n\n\n");
        }
        genfile_c_close(output);
    }
}

static int _gen_fdefault(config_node_t *f, FILE *output, int num)
{
    if (!f->configurable) return 0;
    fprintf(output, "    case conf_numfield_%s_%s:\n", f->parentconf->string, f->string);
    config_node_t *v = find_board_value(f->boardvalues, 0 /*numisnt*/, "unknown");  // XXX numinst
    config_node_t *val = v->val;
    if (val->tag == CONFIG_NODE_TABLEREF) {
        //printf("//table %s col %s inst %d\n", val->tablename, val->colname, numinst);
        config_node_t *v = value_for_table(_fdef_tables, val->tablename, val->colname, 0 /*numinst*/);
        if (v) val = v;
    }
    fprintf(output, "        return %s;\n", val->string);
    return 1;
}
static int _gen_lset(config_node_t *f, FILE *output, int num)
{
    if (1 != f->configurable) return 0;
    fprintf(output, "    case conf_numfield_%s_%s:\n", f->parentconf->string, f->string);
    if (f->parentconf && (f->parentconf->tag == CONFIG_NODE_SUBCONF)) {
        fprintf(output, "        c->%s.%s = v;\n", f->parentconf->string, f->string);
    } else {
        fprintf(output, "        c->%s = v;\n", f->string);
    }
    fprintf(output, "        break;\n");
    return 1;
}


static int _gen_lget(config_node_t *f, FILE *output, int num)
{
    if (1 != f->configurable) return 0;
    fprintf(output, "    case conf_numfield_%s_%s:\n", f->parentconf->string, f->string);
    if (f->parentconf && (f->parentconf->tag == CONFIG_NODE_SUBCONF)) {
        fprintf(output, "        return c->%s.%s;\n", f->parentconf->string, f->string);
    } else {
        fprintf(output, "        return c->%s;\n", f->string);
    }
    return 1;
}

static int _gen_fpropag(config_node_t *f, FILE *output, int num)
{
    if (!f->configurable) return 0;
    fprintf(output, "    case conf_numfield_%s_%s:\n", f->parentconf->string, f->string);
    if (f->parentconf && (f->parentconf->tag == CONFIG_NODE_SUBCONF)) {
        fprintf(output, "        conf->%s.%s = value;\n     break;\n", f->parentconf->string,  f->string);
    } else {
        fprintf(output, "        conf->%s = value;\n        break;\n", f->string);
    }
    return 1;
}

static void gen_field_propag(FILE * output, config_node_t *fields, config_node_t *root)
{
    apply_field(root, fields, 1, _gen_fpropag, output, 0);
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
    config_node_t *v = find_board_value(f->boardvalues, numinst, b ? b->string : "unknown");
    config_node_t *val = v->val;
    if (val->tag == CONFIG_NODE_TABLEREF) {
        //printf("//table %s col %s inst %d\n", val->tablename, val->colname, numinst);
        config_node_t *v = value_for_table(tables, val->tablename, val->colname, numinst);
        if (v) val = v;
    }
   
    fprintf(output, "     .%s = %s,\n", // \t\t// %s:%d,%s:%d tag:%d\n",
        f->string,
        val ? val->string : "missing");
        //b->string, numinst, v->string, v->value, val->tag);
}

// tables management

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
        line = find_by_index(tbl->lines, -1); // use last line
    }
    assert(line->tag == CONFIG_NODE_TABLELINE);
    config_node_t *v = find_by_index(line->lineval, cidx);
    if (!v) {
        fprintf(stderr, "no such column %d in line %d of %s\n", cidx, numinst, tblname);
        return NULL;
    }
    return v;
}

    
static int _genstrfld(config_node_t *f, FILE *output, int num)
{
    if (!f->configurable) return 0;
    fprintf(output, "    } else if (!strcmp(str, \"%s\")) {\n", f->string);
    fprintf(output, "         return conf_numfield_%s_%s;\n", f->parentconf->string,  f->string);
    return 1;
}

static int _genstrfld2(config_node_t *f, FILE *output, int num)
{
    if (!f->configurable) return 0;
    fprintf(output, "    } else if (f == conf_numfield_%s_%s) {\n", f->parentconf->string, f->string);
    fprintf(output, "         return \"%s\";\n", f->string);
    return 1;
}


void generate_cfile_fieldname(config_node_t *root)
{
    config_node_t *node = root->defs;
    for ( ; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag, expect conf");
        }

        char *n = node->string;
        printf("--> config fld str %s\n", n);
        FILE *output = genfile_c_create(node, root, 1);
        fprintf(output, "int conf_%s_fieldnum(const char *str)\n{\n", n);
        fprintf(output, "    if (0) {\n");
        apply_field(root, node->fields, 1, _genstrfld, output, 0);
        fprintf(output, "    }\n    return -1;\n}\n\n");

        fprintf(output, "const char *conf_%s_fieldname(int f)\n{\n", n);
        fprintf(output, "    if (0) {\n");
        apply_field(root, node->fields, 1, _genstrfld2, output, 0);
        fprintf(output, "    }\n    return NULL;\n}\n\n");
        genfile_c_close(output);
    }
} 




