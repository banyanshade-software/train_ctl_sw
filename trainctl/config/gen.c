
#include <memory.h>
#include <stdlib.h>
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
	for ( ; node; node = node->next) {
		if (node->tag != CONFIG_NODE_CONF) {
			error("bad tag");
		}  
		fprintf(output, "typedef struct conf_%s {\n", node->string);
		gen_fields(node->fields, output);
		fprintf(output, "} conf_%s_t;\n", node->string);
		if (!continue_next) break;
	}
}
