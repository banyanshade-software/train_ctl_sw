#ifndef GEN_H
#define GEN_H

void generate_hfiles(config_node_t *root);
void generate_cfile(config_node_t *root, int continue_next, config_node_t *boards);
void generate_cfile_global_propag(config_node_t *root);


#endif
