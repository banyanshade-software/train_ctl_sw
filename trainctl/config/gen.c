
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
		if (node->tag != CONFIG_NODE_FIELD) {
    		error("bad tag");
    	}  
		fprintf(output, "    %s %s%s", node->type, nstar(node->nptr), node->string);
		if (node->bitfield) fprintf(output, ":%d", node->bitfield);
		if (node->array)    fprintf(output, "[%d]", node->array);
		fprintf(output, ";\n");
	}
}

void generate_hfile(config_node_t *node, int continue_next, FILE *output)
{
	fprintf(output, "// this file is generated automatically\n// do not modify\n");
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



static void gen_field_val(FILE *output, config_node_t *f, config_node_t *b, int numinst);
static config_node_t *find_board_value(config_node_t *values, int inst, const char *boardname);

void generate_cfile(config_node_t *node, config_node_t *tables, int continue_next, FILE *output, config_node_t *board)
{
    fprintf(output, "// this file is generated automatically\n// do not modify\n");
    for ( ; node; node = node->next) {
        if (node->tag != CONFIG_NODE_CONF) {
            error("bad tag");
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
					gen_field_val(output, f, b, numinst);
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

static void gen_field_val(FILE *output, config_node_t *f, config_node_t *b, int numinst)
{
	config_node_t *v = find_board_value(f->boardvalues, numinst, b->string);
	fprintf(output, "     .%s = %s \t\t// %s:%d,%s:%d \n",
		f->string,
		v ? v->val->string : "missing",
		b->string, numinst, v->string, v->value);
}

