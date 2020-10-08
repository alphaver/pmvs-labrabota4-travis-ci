#ifndef _FUSE_FS_H
#define _FUSE_FS_H

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>

int read_directory(const char *abs_path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int read_file(const char *abs_path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int open_file(const char *abs_path, struct fuse_file_info *fi);
int get_stat(const char *abs_path, struct stat *stat);
int change_owner(const char *abs_path, uid_t user, gid_t group);

#endif // _FUSE_FS_H
