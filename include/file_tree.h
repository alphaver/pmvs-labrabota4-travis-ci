#ifndef FILETREE_H
#define FILETREE_H

#include "../include/fuse_fs.h"
#include <stdlib.h>

typedef struct file_tree_node_child file_tree_node_child;

typedef struct file_tree_node
{
	char *name;
	void *contents;
	struct stat *file_stat;

	struct file_tree_node *parent;
	file_tree_node_child *children;
}
file_tree_node;

struct file_tree_node_child
{
	file_tree_node *node;
	struct file_tree_node_child *next;
};

file_tree_node * create_file_system();
void destroy_file_system(file_tree_node *root);

int add_file(file_tree_node *root, const file_tree_node *outer_file, const char *directory_abs_path);
file_tree_node * find_file(file_tree_node *root, const char *file_path);

#endif // FILETREE_H
