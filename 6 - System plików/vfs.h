#pragma once

#include <stdio.h>

#define NAME_MAX 32
#define BLOCK_SIZE 2048

struct vfs
{
	FILE * F;
	unsigned int inodes_num;
	struct vfs_inode * inodes;
};
struct vfs_superblock
{
	size_t size;
};

#define VFS_FLAG_IN_USE (1 << 0)
#define VFS_FLAG_IS_START (1 << 1)

struct vfs_inode
{
	unsigned int flags;
	char name[NAME_MAX];
	unsigned int size;
	unsigned int next_node;
};

struct vfs * vfs_create(const char * file_name, const size_t size);
struct vfs * vfs_open(const char * file_name);
void vfs_close(struct vfs * v);
void vfs_delete(const char * file_name);

void vfs_dump(const struct vfs * v);
void vfs_list(const struct vfs * v);
int vfs_copy_to(const struct vfs * v, const char * source_file_name, const char * destination_file_name);
int vfs_copy_from(const struct vfs * v, const char * source_file_name, const char * destination_file_name);
int vfs_delete_file(const struct vfs * v, const char * file_name);

unsigned int vfs_inodes_from_size(const size_t size);
unsigned int vfs_required_inodes_for(const size_t size);
size_t vfs_get_block_position(const struct vfs * v, const size_t inode);