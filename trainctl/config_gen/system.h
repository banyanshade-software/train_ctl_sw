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

#ifndef INCLUDED_SYSTEM_H
#define INCLUDED_SYSTEM_H

#include "utility.h"

#include <stdio.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct system_tag system_t;


                

typedef enum node_type_tag {
    CONFIG_NODE_ROOT, // not used
    CONFIG_NODE_TABLE,
    CONFIG_NODE_TABLELINE,
    CONFIG_NODE_CONF,
    CONFIG_NODE_FIELD,
    CONFIG_NODE_SUBCONF,
    CONFIG_NODE_SUBREF,
	CONFIG_NODE_BOARDVAL,
	CONFIG_NODE_VALUE,
	CONFIG_NODE_TABLEREF,

    CONFIG_NODE_ATTR_LOCAL,
    CONFIG_NODE_ATTR_CODE,
    CONFIG_NODE_ATTR_STORETYPE,
    CONFIG_NODE_ATTR_STORENUM,

	// transiant use only
    CONFIG_NODE_IDENT,	
    CONFIG_NODE_INT,
	
	// from main.c
	CONFIG_NODE_BOARD

} node_type_t;


	
typedef struct config_node {
    range_t range; /* the byte range in the source text */
    system_t *system; /* the system that manages this AST node */
    enum node_type_tag tag;
    struct config_node *next;

    char *string;
    int value;
    char *hcode;
    char *ccode;

    union {
		struct { // root node CONFIG_NODE_ROOT
			struct config_node *defs;
			struct config_node *subdefs;
            struct config_node *tables;
            struct config_node *globattrib;
        };
		struct { // config CONFIG_NODE_CONF or CONFIG_NODE_SUBCONF
			struct config_node *fields;
            struct config_node *numinst;
            struct config_node *attr;
		};
		struct { // field definition CONFIG_NODE_FIELD
			int configurable;
			int bitfield;
			int array;
			int nptr;
			char *type;
			struct config_node *boardvalues;
			struct config_node *parentconf;
		};
		struct { // per board value CONFIG_NODE_BOARDVAL
			struct config_node *val;
		};
        struct { // table CONFIG_NODE_TABLE
            struct config_node *coldef;
            struct config_node *lines;
        };
        struct { // CONFIG_NODE_TABLELINE
            struct config_node *lineval;
        };
        struct { // CONFIG_NODE_TABLEREF
            char *tablename;
            char *colname;
        };
        struct { // CONFIG_NODE_ATTR_LOCAL
            struct config_node *attrnode; 
        };
    };
} config_node_t;


typedef enum syntax_error_tag {
    SYNTAX_ERROR_IF_WITHOUT_CONDITION,
    SYNTAX_ERROR_IF_WITHOUT_STATEMENT,
    SYNTAX_ERROR_ELSE_WITHOUT_STATEMENT,
    SYNTAX_ERROR_LONE_ELSE,
    SYNTAX_ERROR_WHILE_WITHOUT_CONDITION,
    SYNTAX_ERROR_WHILE_WITHOUT_STATEMENT,
    SYNTAX_ERROR_DO_WITHOUT_STATEMENT,
    SYNTAX_ERROR_DO_WITHOUT_WHILE,
    SYNTAX_ERROR_NO_ENDING_SEMICOLON,
    SYNTAX_ERROR_UNCLOSED_COMMENT_BLOCK,
    SYNTAX_ERROR_UNEXPECTED_TOKEN,
    SYNTAX_ERROR_UNKNOWN
} syntax_error_t;

struct system_tag {
    struct system_input_tag {
        const char *path; /* the source file path */
        FILE *file; /* the source file pointer */
        char_array_t text; /* the source text */
        size_t_array_t line; /* the byte positions of the line head in the source text */
        size_t ecount; /* the error count */
    } source;
    struct system_managed_tag {
        config_node_t *first; /* the first managed AST node */
        config_node_t *last; /* the last managed AST node */
    } managed;
    jmp_buf jmp;
};

void system__initialize(system_t *obj);
void system__finalize(system_t *obj);

void *system__allocate_memory(system_t *obj, size_t size);
void *system__reallocate_memory(system_t *obj, void *ptr, size_t size);
void system__deallocate_memory(system_t *obj, void *ptr);

void system__open_source_file(system_t *obj, const char *path); /* the standard input if path == NULL */
void system__close_source_file(system_t *obj);
int system__read_source_file(system_t *obj);

void system__handle_syntax_error(system_t *obj, syntax_error_t error, range_t range);


void system__destroy_all_ast_nodes(system_t *obj);

void system__dump_ast(system_t *obj, config_node_t *root);

//void ast_node__prepend_child(ast_node_t *obj, ast_node_t *node);
//void ast_node__append_child(ast_node_t *obj, ast_node_t *node);
void ast_node__destroy(config_node_t *obj);


config_node_t *create_config_node(system_t *obj, node_type_t type, range_t range);
config_node_t *create_config_node_text(system_t *obj, node_type_t type, range_t range);
config_node_t *create_config_node_string(system_t *obj, node_type_t type, const char *str);
config_node_t *create_config_node_int(system_t *obj, node_type_t type, range_t range, int v);
config_node_t *create_config_node_intstr(system_t *obj, node_type_t type, range_t range, int base);
void config_node_append(config_node_t *node, config_node_t *end);

#ifdef __cplusplus
}
#endif

#endif /* !INCLUDED_SYSTEM_H */
