/*
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef _MSC_VER
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS /* suppress the warning of fopen() use */
#endif /* _MSC_VER */

#include <memory.h>
#include <stdlib.h>
#include "system.h"


#ifndef ERROR_MAX
#define ERROR_MAX 4 /* the maximum number of errors displayed at a time */
#endif

static void append_text_character_(system_t *obj, char c) {
    const size_t n = obj->source.text.n;
    if (!char_array__resize(&(obj->source.text), n + 1)) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    obj->source.text.p[n] = (char)c;
}

static void append_line_head_(system_t *obj, size_t h) {
    const size_t n = obj->source.line.n;
    if (!size_t_array__resize(&(obj->source.line), n + 1)) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    obj->source.line.p[n] = h;
}

static size_t count_characters_(const char *buf, size_t start, size_t end) {
    /* UTF-8 multibyte character support but without checking UTF-8 validity */
    size_t n = 0, i = start;
    while (i < end) {
        const int c = (int)(unsigned char)buf[i];
        if (c == 0) break;
        n++;
        i += (c < 0x80) ? 1 : ((c & 0xe0) == 0xc0) ? 2 : ((c & 0xf0) == 0xe0) ? 3 : ((c & 0xf8) == 0xf0) ? 4 : /* invalid code */ 1;
    }
    return n;
}

static void compute_line_and_column_(system_t *obj, size_t pos, size_t *line, size_t *column) {
    size_t i;
    for (i = 1; i < obj->source.line.n; i++) {
        if (pos < obj->source.line.p[i]) break;
    }
    if (line) *line = i;
    if (column) *column = count_characters_(obj->source.text.p, obj->source.line.p[i - 1], pos) + 1;
}

void system__initialize(system_t *obj) {
    obj->source.path = NULL;
    obj->source.file = NULL;
    char_array__initialize(&(obj->source.text));
    size_t_array__initialize(&(obj->source.line));
    obj->source.ecount = 0;
    obj->managed.first = NULL;
    obj->managed.last = NULL;
    append_line_head_(obj, 0);
}

void system__finalize(system_t *obj) {
    if (obj->source.file != NULL && obj->source.file != stdin) fclose(obj->source.file);
    char_array__finalize(&(obj->source.text));
    size_t_array__finalize(&(obj->source.line));
    system__destroy_all_ast_nodes(obj);
}

void *system__allocate_memory(system_t *obj, size_t size) {
    void *const p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    memset(p, 0, size);
    return p;
}

void *system__reallocate_memory(system_t *obj, void *ptr, size_t size) {
    void *const p = realloc(ptr, size);
    if (p == NULL) {
        fprintf(stderr, "FATAL: Out of memory\n");
        longjmp(obj->jmp, 1); /* never returns */
    }
    return p;
}

void system__deallocate_memory(system_t *obj, void *ptr) {
    free(ptr);
}

void system__open_source_file(system_t *obj, const char *path) {
    if (path != NULL) {
        FILE *const f = fopen(path, "rt");
        if (f == NULL) {
            fprintf(stderr, "FATAL: Cannot open file '%s'\n", path);
            longjmp(obj->jmp, 1); /* never returns */
        }
        obj->source.path = path;
        obj->source.file = f;
    }
    else {
        obj->source.path = NULL;
        obj->source.file = stdin;
    }
}

void system__close_source_file(system_t *obj) {
    if (obj->source.file != NULL && obj->source.file != stdin) {
        if (fclose(obj->source.file) == EOF) {
            fprintf(stderr, "FATAL: Error occurred while closing file '%s'\n", obj->source.path);
            longjmp(obj->jmp, 1); /* never returns */
        }
    }
    obj->source.path = NULL;
    obj->source.file = NULL;
}

int system__read_source_file(system_t *obj) {
    const int c = fgetc(obj->source.file);
    if (c != EOF) {
        append_text_character_(obj, (char)c);
        if (c == '\r') {
            append_line_head_(obj, obj->source.text.n);
        }
        else if (c == '\n') {
            if (obj->source.text.n >= 2 && obj->source.text.p[obj->source.text.n - 2] == '\r') {
                obj->source.line.p[obj->source.line.n - 1] = obj->source.text.n;
            }
            else {
                append_line_head_(obj, obj->source.text.n);
            }
        }
    }
    else {
        if (ferror(obj->source.file)) {
            if (obj->source.path != NULL) {
                fprintf(stderr, "FATAL: Error occurred while reading file '%s'\n", obj->source.path);
            }
            else {
                fprintf(stderr, "FATAL: Error occurred while reading standard input\n");
            }
            longjmp(obj->jmp, 1); /* never returns */
        }
    }
    return c;
}

// -------

config_node_t *create_config_node(system_t *obj, node_type_t type, range_t range)
{
    config_node_t *const node = (config_node_t *)system__allocate_memory(obj, sizeof(config_node_t));
    node->tag = type;
    node->range = range;
    node->system = obj;
    return node;
}

config_node_t *create_config_node_text(system_t *obj, node_type_t type, range_t range) 
{
    config_node_t *node = create_config_node(obj, type, range);
    node->string = strndup(obj->source.text.p + range.min, range.max - range.min);
    //printf("id : <%s>\n", node->string);
    return node;
}

config_node_t *create_config_node_intstr(system_t *obj, node_type_t type, range_t range, int base) 
{
    config_node_t *node = create_config_node(obj, type, range);
    node->string = strndup(obj->source.text.p + range.min, range.max - range.min);
	node->value = strtol(node->string, NULL, base);
    //printf("val : <%d>\n", node->value);
    return node;
}


config_node_t *create_config_node_int(system_t *obj, node_type_t type, range_t range, int v)
{
	config_node_t *node = create_config_node(obj, type, range);
    node->value = v;
    return node;
}

config_node_t *create_config_node_string(system_t *obj, node_type_t type, const char *str)
{
	config_node_t *node = create_config_node(obj, type, range__void());
	node->string = strdup(str);
	return node;
}



void config_node_append(config_node_t *node, config_node_t *end)
{
	for (; node->next; node=node->next);
	node->next = end;
}



// -------

void system__destroy_all_ast_nodes(system_t *obj) {
    while (obj->managed.first != NULL) {
        ast_node__destroy(obj->managed.first);
    }
}


void ast_node__destroy(config_node_t *obj) {
/*
    if (obj->parent != NULL) {
        if (obj->sibling.prev != NULL) {
            obj->sibling.prev->sibling.next = obj->sibling.next;
        }
        else {
            obj->parent->child.first = obj->sibling.next;
        }
        if (obj->sibling.next != NULL) {
            obj->sibling.next->sibling.prev = obj->sibling.prev;
        }
        else {
            obj->parent->child.last = obj->sibling.prev;
        }
        obj->parent->arity--;
    }
    while (obj->child.first != NULL) {
        ast_node_t *const child = obj->child.first;
        obj->child.first = child->sibling.next;
        child->sibling.next = NULL;
        child->sibling.prev = NULL;
        child->parent = NULL;
    }
    {
        if (obj->managed.prev != NULL) {
            obj->managed.prev->managed.next = obj->managed.next;
        }
        else {
            obj->system->managed.first = obj->managed.next;
        }
        if (obj->managed.next != NULL) {
            obj->managed.next->managed.prev = obj->managed.prev;
        }
        else {
            obj->system->managed.last = obj->managed.prev;
        }
    }
    system__deallocate_memory(obj->system, obj);
*/
}

void system__handle_syntax_error(system_t *obj, syntax_error_t error, range_t range) {
    size_t line, col;
    compute_line_and_column_(obj, range.min, &line, &col);
    obj->source.ecount++;
    switch (error) {
    case SYNTAX_ERROR_IF_WITHOUT_CONDITION:
        fprintf(stderr, "ERROR: line %zu, column %zu: Condition missing after 'if'\n", line, col);
        break;
    case SYNTAX_ERROR_IF_WITHOUT_STATEMENT:
        fprintf(stderr, "ERROR: line %zu, column %zu: Statement missing after 'if' condition\n", line, col);
        break;
    case SYNTAX_ERROR_ELSE_WITHOUT_STATEMENT:
        fprintf(stderr, "ERROR: line %zu, column %zu: Statement missing after 'else'\n", line, col);
        break;
    case SYNTAX_ERROR_LONE_ELSE:
        fprintf(stderr, "ERROR: line %zu, column %zu: 'else' without corresponding 'if'\n", line, col);
        break;
    case SYNTAX_ERROR_WHILE_WITHOUT_CONDITION:
        fprintf(stderr, "ERROR: line %zu, column %zu: Condition missing after 'while'\n", line, col);
        break;
    case SYNTAX_ERROR_WHILE_WITHOUT_STATEMENT:
        fprintf(stderr, "ERROR: line %zu, column %zu: Statement missing after 'while' condition\n", line, col);
        break;
    case SYNTAX_ERROR_DO_WITHOUT_STATEMENT:
        fprintf(stderr, "ERROR: line %zu, column %zu: Statement missing after 'do'\n", line, col);
        break;
    case SYNTAX_ERROR_DO_WITHOUT_WHILE:
        fprintf(stderr, "ERROR: line %zu, column %zu: 'while' missing after 'do' statement\n", line, col);
        break;
    case SYNTAX_ERROR_NO_ENDING_SEMICOLON:
        fprintf(stderr, "ERROR: line %zu, column %zu: Ending semicolon missing\n", line, col);
        break;
    case SYNTAX_ERROR_UNCLOSED_COMMENT_BLOCK:
        fprintf(stderr, "ERROR: line %zu, column %zu: Unclosed comment block\n", line, col);
        break;
    case SYNTAX_ERROR_UNEXPECTED_TOKEN:
        fprintf(stderr, "ERROR: line %zu, column %zu: Unexpected token '%.*s'\n", line, col, (int)(range.max - range.min), obj->source.text.p + range.min);
        break;
    case SYNTAX_ERROR_UNKNOWN:
        fprintf(stderr, "ERROR: line %zu, column %zu: Unknown error\n", line, col);
        break;
    default:
        fprintf(stderr, "ERROR: line %zu, column %zu: Undefined internal error\n", line, col);
    }
    if (obj->source.ecount >= ERROR_MAX) longjmp(obj->jmp, 1); /* never returns */
}


static void dump_ast_(system_t *obj, config_node_t *node, int level) {
	if (!node) return;
    const char *type = "UNKNOWN";
    switch (node->tag) {
	case CONFIG_NODE_ROOT:		type="ROOT";	break;
    case CONFIG_NODE_IDENT:		type="IDENT";	break;
    case CONFIG_NODE_CONF:		type="CONF";	break;
	case CONFIG_NODE_FIELD:		type="FIELD";	break;
	case CONFIG_NODE_TABLE:		type="TABLE";	break;
	case CONFIG_NODE_TABLELINE:	type="LINE";	break;
	case CONFIG_NODE_TABLEREF:	type="TREF";	break;
	case CONFIG_NODE_INT:		type="INT";		break;
    case CONFIG_NODE_SUBCONF:   type="SUBCONF"; break;
    case CONFIG_NODE_SUBREF:    type="SREF";    break;
    case CONFIG_NODE_ATTR_CODE: type="CODE";    break;
    case CONFIG_NODE_ATTR_LOCAL: type="LOCATTR";    break;
	default: break;
	}

	char *n = "()";
    if (node->string) n = node->string;

	printf("%*s%s: string=%s", 2 * level, "", type, n);
	switch (node->tag) {
	default: printf("\n"); break;
	case CONFIG_NODE_FIELD:
		if (node->bitfield) printf(":%d", node->bitfield);
		if (node->nptr) printf(",nptr=%d,", node->nptr);
		if (node->type) printf(",type=%s,", node->type);
		if (node->array) printf("[%d]", node->array);
		if (node->configurable) printf(" (USER)");
		printf("\n");
		break;
    case CONFIG_NODE_SUBCONF:
    case CONFIG_NODE_CONF:
		printf("\n");
        dump_ast_(obj, node->fields, level+1);
		printf("\n");
        break;
    case CONFIG_NODE_TABLE:
        printf("\n%*scols:\n", 2*level, "");
        dump_ast_(obj, node->coldef, level+1);
        printf("%*slines:\n", 2*level, "");
        dump_ast_(obj, node->lines, level+1);
		printf("\n");
        break;
    case CONFIG_NODE_TABLELINE:
        printf("\n");
        dump_ast_(obj, node->lineval, level+1);
        break;
	}

	dump_ast_(obj, node->next, level);
}


void system__dump_ast(system_t *obj, config_node_t *root) {
    dump_ast_(obj, root, 0);
}

