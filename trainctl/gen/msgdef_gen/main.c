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
#include "genmsg.h"
#include "gen.h"

/*
static config_node_t *find_by_name_and_type(config_node_t *node, const char *s, node_type_t t)
{
    for ( ; node; node = node->next) {
        if (strcmp(node->string, s)) continue;
        if (node->tag != t) continue;
        return node;
    }
    return NULL;
}
*/

int main(int argc, char **argv) 
{
    if (argc !=3) {
        fprintf(stderr, "usage: %s gen deffile\n", argv[0]);
        fprintf(stderr, "  gen values :\n    h   :  generate msgdef.h\n");
        return 1; /* usage error */
    }
    const char *path = argv[2];
    if (!strcmp(path, "-")) path = NULL; // stdin

    const char *gen = argv[1];

    int ret = 0;
    system_t system;
    msgdef_context_t *parser = NULL;
    if (setjmp(system.jmp) == 0) {
        system__initialize(&system);
        system__open_source_file(&system, path);
        parser = msgdef_create(&system);
        msg_node_t *ast;
        const int b = msgdef_parse(parser, &ast);
        if (system.source.ecount > 0) longjmp(system.jmp, 1); /* never returns */
        if (b) {
            ret = 10; /* internal error */
            fprintf(stderr, "FATAL: Internal error\n");
                /* <-- input text remaining due to incompleteness of the grammar */
        } else {
	        fprintf(stderr, "parsed, ast=%p\n", ast);
           	system__dump_ast(&system, ast);

            if (0) {
            } else if (!strcmp(gen, "h")) {
                generate_msgdef_header(ast);
            } else {
                fprintf(stderr, "bad gen parameteri '%s'\n", gen);
                exit(2);
            }
			/*generate_hfiles(root, bm);
			generate_cfile(root, 1, bm);
        
            generate_cfile_global_propag(root);    
            generate_cfile_fieldname(root);
            */
        }
    } else {
        ret = 2; /* error during parsing */
    }
    msgdef_destroy(parser);

    system__finalize(&system); /* all system resources are freed */
    return ret;
}
