#include "../include/file_tree.h"

file_tree_node * traverse(file_tree_node *root, const char *rel_path)
{
	if (!rel_path[0])
		return root;

	char *inner_path = malloc(strlen(rel_path) + 2);
	
	strcpy(inner_path, rel_path);
	strcat(inner_path, "/");
	char *token = strtok(inner_path, "/");
	file_tree_node *curr = root;	
	while (token)
	{
		file_tree_node_child *child = curr->children;
		while (child)
		{
			if (!strcmp(child->node->name, token))	
			{
				curr = child->node;
				break;
			}	
			child = child->next;
		}
		if (!child)
		{
			free(inner_path);
			return NULL;
		}

		token = strtok(NULL, "/");
	}

	free(inner_path);
	return curr;
}

file_tree_node * duplicate_node_info(const file_tree_node *node)
{
	file_tree_node *result = malloc(sizeof(file_tree_node));
	
	result->name = malloc(strlen(node->name) + 1);
	result->file_stat = malloc(sizeof(struct stat));
        result->contents = malloc(node->file_stat->st_size);	
	result->children = result->parent = NULL;

	strcpy(result->name, node->name);
	memcpy(result->file_stat, node->file_stat, sizeof(struct stat));
	memcpy(result->contents, node->contents, node->file_stat->st_size);

	return result;
}

void release_file_node(file_tree_node *node)
{
	free(node->contents);
	free(node->name);
	free(node->file_stat);
	free(node);
}

file_tree_node * create_file_system() 
{
	file_tree_node *result = malloc(sizeof(file_tree_node));
        if (!result)
                return NULL;

	result->name = malloc(1);
        result->file_stat = malloc(sizeof(struct stat));
        if (!result->name || !result->file_stat)
        {
		if (result->name)
			free(result->name);
                if (result->file_stat)
                        free(result->file_stat);
                free(result);
                return NULL;
        }
	
	result->name[0] = '\0';
	result->file_stat->st_mode = S_IFDIR | 0777;
	result->file_stat->st_nlink = 2;

	result->contents = NULL;
	result->children = result->parent = NULL;

	return result;
}

int add_file(file_tree_node *root, const file_tree_node *outer_file, const char *directory_abs_path)
{
	file_tree_node *node = traverse(root, directory_abs_path + 1);
	if (!node)
		return -ENOENT;
	if (!S_ISDIR(node->file_stat->st_mode))
		return -ENOTDIR;

	file_tree_node *file = duplicate_node_info(outer_file);
	file_tree_node_child *child = node->children, *start_child;
	while (child)
	{
		if (!strcmp(child->node->name, file->name))
			return -EEXIST;
		child = child->next;
	}

	start_child = node->children;
	child = malloc(sizeof(file_tree_node_child));
	child->node = file;
	child->next = start_child;

	node->children = child;
	file->parent = node;	

	return 0;
}

file_tree_node *find_file(file_tree_node *root, const char *file_path)
{
	return traverse(root, file_path + 1);
}

void destroy_file_system(file_tree_node *root)
{
	file_tree_node_child *child = root->children, *temp_child;
	while (child)
	{
		temp_child = child;
		child = child->next;

		destroy_file_system(temp_child->node);
		free(temp_child);
	}
	release_file_node(root);
}
