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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "system.h"
#include "config.h"

#include "gen.h"


static config_node_t *find_by_name_and_type(config_node_t *node, const char *s, node_type_t t)
{
    for ( ; node; node = node->next) {
        if (strcmp(node->string, s)) continue;
        if (node->tag != t) continue;
        return node;
    }
    return NULL;
}


int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "ERROR: Too many arguments\n");
        return 1; /* usage error */
    }
    const char *path = (argc > 1) ? argv[1] : NULL;
    int ret = 0;
    system_t system;
    cnfparse_context_t *parser = NULL;
    if (setjmp(system.jmp) == 0) {
        system__initialize(&system);
        system__open_source_file(&system, path);
        parser = cnfparse_create(&system);
        config_node_t *ast;
        const int b = cnfparse_parse(parser, &ast);
        if (system.source.ecount > 0) longjmp(system.jmp, 1); /* never returns */
        if (b) {
            ret = 10; /* internal error */
            fprintf(stderr, "FATAL: Internal error\n");
                /* <-- input text remaining due to incompleteness of the grammar */
        }
        else {
	    fprintf(stderr, "parsed\n");
           	system__dump_ast(&system, ast);
            config_node_t *root = create_config_node(&system, CONFIG_NODE_ROOT, range__void());
            config_node_t *next = NULL;
            config_node_t *loc = NULL;
            for (config_node_t *n = ast; n; n = next) {
                next = n->next;
                n->next = NULL;
                switch (n->tag) {
                case CONFIG_NODE_TABLE:
                    n->next = root->tables;
                    root->tables = n;
                    break;
                case CONFIG_NODE_CONF:
                    n->next = root->defs;
                    root->defs = n;
                    break;
                case CONFIG_NODE_SUBCONF:
                    n->next = root->subdefs;
                    root->subdefs = n;
                    break;
                case CONFIG_NODE_ATTR_CODE:
                    n->next = root->globattrib;
                    root->globattrib = n;
                    break;
                case CONFIG_NODE_ATTR_LOCAL:
                    n->next = loc;
                    loc = n;
                    // handle it in 2nd pass
                    break;
                default:
                    fprintf(stderr, "bad top level tag %d\n", n->tag);
                    exit(1);
                    break;
                }
            }
            for (config_node_t *n = loc; n; n = n->next) {
                config_node_t *c;
                switch (n->tag) {
                default:
                    break;
                case CONFIG_NODE_ATTR_LOCAL:
                    printf("proc CONFIG_NODE_ATTR_LOCAL\n");
                    c = find_by_name_and_type(root->defs, n->string, CONFIG_NODE_CONF);
                    if (!c) {
                        fprintf(stderr, "no such config '%s'\n", n->string);
                        exit(1);
                    }
                    config_node_t *a = n->attrnode;
                    assert(a);
                    assert(!a->next);
                    a->next = c->attr;
                    c->attr = a;
                    break;
                }
            }


            config_node_t *bm;
            config_node_t *pbm = NULL;
            static const char *boards[] = { "simu", "main_zero", "switcher", "dispatcher", "mainV0", "mainV04", "ui", "unit_test",  NULL};
            //static const char *boards[] = { "simu", "main_zero", "switcher", "dispatcher", "main", NULL};
            for (int i=0; boards[i]; i++) {
                printf(".......%s\n", boards[i]);
			    bm = create_config_node_string(&system, CONFIG_NODE_BOARD, boards[i]);
                bm->next = pbm;
                pbm = bm;
            }

			generate_hfiles(root, bm);
			generate_cfile(root, 1, bm);
        
            generate_cfile_global_propag(root);    
            generate_cfile_fieldname(root);
        }
    }
    else {
        ret = 2; /* error during parsing */
    }
    cnfparse_destroy(parser);

    system__finalize(&system); /* all system resources are freed */
    return ret;
}
