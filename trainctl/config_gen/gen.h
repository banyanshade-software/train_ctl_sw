#ifndef GEN_H
#define GEN_H

void generate_hfile(config_node_t *root, int continue_next);
void generate_cfile(config_node_t *root, int continue_next, config_node_t *boards);

#endif
