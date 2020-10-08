#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS 64

#include "../include/fuse_fs.h"
#include "../include/file_tree.h"

#define LAST_TWO_DIGITS 6

static file_tree_node *file_system_root;

int get_stat(const char *abs_path, struct stat *st)
{	
	file_tree_node *node = find_file(file_system_root, abs_path);
	if (!node)
		return -ENOENT;	
	
	memcpy(st, node->file_stat, sizeof(struct stat));
	return 0;
}

int read_directory(const char *abs_path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	file_tree_node *node = find_file(file_system_root, abs_path);
	if (!node)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	file_tree_node_child *child = node->children;
	while (child)
	{
		filler(buf, child->node->name, NULL, 0);
		child = child->next;
	}

	return 0;
}

int read_file(const char *abs_path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
	file_tree_node *node = find_file(file_system_root, abs_path);
	if (!node)
		return -ENOENT;

	size_t file_length = node->file_stat->st_size;

	if (offset < file_length) 
	{
		if (offset + size > file_length)
			size = file_length - offset;
		memcpy(buffer, node->contents + offset, size);
		return size;
	}

	return 0;
}

int change_owner(const char *abs_path, uid_t user, gid_t group)
{
	file_tree_node *node = find_file(file_system_root, abs_path);
	if (!node)
		return -ENOENT;	
	
	node->file_stat->st_uid = user;
	node->file_stat->st_gid = group;

	return 0;
}

void fill_node_info(file_tree_node *node, const char *name, const void *contents, size_t contents_size, mode_t mode)
{
	node->name = malloc(strlen(name) + 1);
	node->file_stat = malloc(sizeof(struct stat));

	strcpy(node->name, name);
	node->file_stat->st_mode = mode;
	node->file_stat->st_uid = getuid();
	node->file_stat->st_gid = getgid();
	node->file_stat->st_nlink = 1;
	if (S_ISDIR(mode))
		++node->file_stat->st_nlink;

	if (contents)
	{
		node->contents = malloc(contents_size);

		memcpy(node->contents, contents, contents_size);
		node->file_stat->st_size = contents_size;
	}
	else
		node->contents = NULL;

	node->children = node->parent = NULL;
}

void clean_node_info(file_tree_node *node)
{
	free(node->contents);
	free(node->file_stat);
	free(node->name);
}

void populate_file_system()
{
	const char *dir_to_add[8] = 
	{
		"/", "/", "/", "/",
		"/baz", "/baz", "/foo", "/foo"
	};
		
	file_tree_node nodes[8];
	fill_node_info(nodes, "bin", NULL, 0, S_IFDIR | 0022);
	fill_node_info(nodes + 1, "bar", NULL, 0, S_IFDIR | 0555);
	fill_node_info(nodes + 2, "baz", NULL, 0, S_IFDIR | 0744);
	fill_node_info(nodes + 3, "foo", NULL, 0, S_IFDIR | 0771);

	const char *readme_text = "Student Daniil Lebedev, 1823106";
	const char *example_text = "Hello world! Student Daniil Lebedev, group 13, task 6";
	fill_node_info(nodes + 4, "readme.txt", readme_text, strlen(readme_text), S_IFREG | 0644);
	fill_node_info(nodes + 5, "example", example_text, strlen(example_text), S_IFREG | 0777);

	FILE *pwd_file = fopen("/usr/bin/pwd", "rb");
	fseek(pwd_file, 0, SEEK_END);
	long pwd_size = ftell(pwd_file);
	fseek(pwd_file, 0, SEEK_SET);
	char *pwd_buffer = malloc(pwd_size);
	fread(pwd_buffer, 1, pwd_size, pwd_file);
	fclose(pwd_file);

	fill_node_info(nodes + 6, "pwd", pwd_buffer, pwd_size, S_IFREG | 0777);

	const char *test_lipsum_fmt = "(%i) Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n";
	char *test_buffer = malloc(strlen(test_lipsum_fmt) * LAST_TWO_DIGITS + 1);
	int offset = 0;
	for (int i = 0; i < LAST_TWO_DIGITS; ++i)
		offset += sprintf(test_buffer + offset, test_lipsum_fmt, i);

	fill_node_info(nodes + 7, "test.txt", test_buffer, strlen(test_buffer), S_IFREG | 0000);

	for (int i = 0; i < 8; ++i)
	{
		add_file(file_system_root, nodes + i, dir_to_add[i]);
		clean_node_info(nodes + i);
	}

	free(test_buffer);
	free(pwd_buffer);
}

void fuse_destroy(struct fuse *f)
{
	destroy_file_system(file_system_root);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		puts("Must have exactly one parameter: the mount point!");
		return 0;
	}

	file_system_root = create_file_system();

	populate_file_system();

	struct fuse_operations operations = 
	{
		.getattr = get_stat,
		.readdir = read_directory,
		.read = read_file,
		.chown = change_owner,
		.destroy = fuse_destroy
	};

	return fuse_main(argc, argv, &operations, NULL);
}
