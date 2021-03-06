#ifndef DICO_FINDER_H
# define DICO_FINDER_H

struct be_node *dico_find(struct be_node *node, const char *key);
char *dico_find_str(struct be_node *dico, const char *key);
long long dico_find_int(struct be_node *dico, const char *key);

#endif
