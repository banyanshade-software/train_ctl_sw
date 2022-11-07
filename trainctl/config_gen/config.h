/* A packrat parser generated by PackCC 1.8.0 */

#ifndef PCC_INCLUDED_CONFIG_H
#define PCC_INCLUDED_CONFIG_H

#line 11 "config.peg"
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
#line 10 "config.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct cnfparse_context_tag cnfparse_context_t;

cnfparse_context_t *cnfparse_create(system_t *auxil);
int cnfparse_parse(cnfparse_context_t *ctx, config_node_t **ret);
void cnfparse_destroy(cnfparse_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !PCC_INCLUDED_CONFIG_H */
