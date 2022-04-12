#ifndef NODE_UTIL_H
#define NODE_UTIL_H

// find by name, optionnaly returns position index in pn
config_node_t *find_by_name(config_node_t *node, const char *s, int *pn);

// find by name and type
// ignore type if t=-1
// ignore name if null
config_node_t *find_by_name_and_type(config_node_t *node, const char *s, node_type_t t);

// find the nth element in list
config_node_t *find_by_index(config_node_t *node, int idx);

// apply func() to all field, optionaly recursing in SUBREF
//      recsubref=0 : ignore SUBREF
//      recsubref=1 : recurse in subrefs
//      recusbref=-1 : only apply func to subref
void apply_field(config_node_t *root, config_node_t *fields, int recsubref, void (*func)(config_node_t *n, FILE *output, void *priv), FILE *
output, void *priv);


#endif
