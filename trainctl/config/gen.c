
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



void generate_cfile(config_node_t *node, int continue_next, FILE *output, config_node_t *board)
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
			fprintf(output, "#endif // TRN_BOARD_%s\n\n\n", brduc);
		}
	}
}

