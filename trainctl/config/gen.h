#ifndef GEN_H
#define GEN_H

void generate_hfile(config_node_t *node, int continue_next, FILE *output);
void generate_cfile(config_node_t *node, int continue_next, FILE *outputi, config_node_t *boards);

#endif
