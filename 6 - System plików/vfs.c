#include "vfs.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct vfs * vfs_create(const char * file_name, const size_t size)
{
	FILE * F;
	
	char zero_block[128];
	
	size_t bytes_remaining;
	size_t bytes_to_write;
	
	struct vfs_superblock sb;
	unsigned int inodes_num;
	
	struct vfs_inode * inodes;
	unsigned int I;
	
	struct vfs * v;
	
	
	F = fopen(file_name, "wb");
	if(!F)
		return NULL;
	
	memset(zero_block, 0, sizeof(zero_block));
	bytes_remaining = size;
	
	while(bytes_remaining > 0)
	{
		bytes_to_write = sizeof(zero_block);
		if(bytes_to_write > bytes_remaining)
			bytes_to_write = bytes_remaining;
		
		fwrite(zero_block, 1, bytes_to_write, F);
		
		bytes_remaining -= bytes_to_write;
	}
	
	fseek(F, 0, SEEK_SET);
	sb.size = size;
	
	fwrite(&sb, sizeof(sb), 1, F);
	
	inodes_num = vfs_inodes_from_size(size);
	
	inodes = malloc(sizeof(struct vfs_inode) * inodes_num);
	for(I = 0; I < inodes_num; I++)
		inodes[I].flags = 0;
	
	v = malloc(sizeof(struct vfs));
	v->F = F;
	v->inodes_num = inodes_num;
	v->inodes = inodes;
	
	return v;
}
struct vfs * vfs_open(const char * file_name)
{
	FILE * F;
	
	size_t size_file;
	struct vfs_superblock sb;
	unsigned int inodes_num;
	
	struct vfs_inode * inodes;
	struct vfs * v;
	
	F = fopen(file_name, "r+b");
	if(!F)
		return NULL;
	
	fseek(F, 0, SEEK_END);
	size_file = ftell(F);
	
	if(size_file < sizeof(struct vfs_superblock))
	{
		fclose(F);
		return NULL;
	}
	fseek(F, 0, SEEK_SET);
	
	if(fread(&sb, sizeof(sb), 1, F) <= 0)
	{
		fclose(F);
		return NULL;
	}
	
	if(sb.size != size_file)
	{
		fclose(F);
		return NULL;
	}
	
	inodes_num = vfs_inodes_from_size(size_file);
	
	inodes = malloc(sizeof(struct vfs_inode) * inodes_num);
	if(fread(inodes, sizeof(struct vfs_inode), inodes_num, F) <= 0)
	{
		fclose(F);
		free(inodes);
		return NULL;
	}
	
	v = malloc(sizeof(struct vfs));
	v->F = F;
	v->inodes_num = inodes_num;
	v->inodes = inodes;
	
	return v;
}
void vfs_close(struct vfs * v)
{	
	fseek(v->F, sizeof(struct vfs_superblock), SEEK_SET);
	fwrite(v->inodes, sizeof(struct vfs_inode), v->inodes_num, v->F);
	
	fclose(v->F);
	free(v->inodes);
	free(v);
	v = NULL;
}
void vfs_delete(const char * file_name)
{
	unlink(file_name);
}






void vfs_dump(const struct vfs * v)
{
	unsigned int I;
	unsigned int free_nodes;
	
	printf("== Inodes:\n");
	for(I = 0; I < v->inodes_num; I++)
	{
		printf("= ID: %d\n", I);
		printf("= Flags: %d\n", v->inodes[I].flags);
		printf("= Name: %s\n", v->inodes[I].name);
		printf("= Data size: %d\n", v->inodes[I].size);
		printf("= Next Node: %d\n", v->inodes[I].next_node);
		printf("\n");
	}
	
	printf("== Usage log:\n");
	free_nodes = 0;
	for(I = 0; I < v->inodes_num; I++)
	{
		if(!(v->inodes[I].flags & VFS_FLAG_IN_USE))
			free_nodes++;
			
		printf("%c", (v->inodes[I].flags & VFS_FLAG_IN_USE ? (v->inodes[I].flags & VFS_FLAG_IS_START ? '*' : '+') : '-'));
	}
	
	printf("\n");
	printf("== Free inodes: %d/%d\n", free_nodes, v->inodes_num);
}
void vfs_list(const struct vfs * v)
{
	unsigned int I;
	for(I = 0; I < v->inodes_num; I++)
	{
		if((v->inodes[I].flags & VFS_FLAG_IN_USE) && (v->inodes[I].flags & VFS_FLAG_IS_START))
		{
			printf("File: %s @%d\n", v->inodes[I].name, I);
		}
	}
}
int vfs_copy_to(const struct vfs * v, const char * source_file_name, const char * destination_file_name)
{
	FILE * source_file;
	size_t source_file_length;
	unsigned int required_inodes;
	unsigned int * nodes_queue;
	unsigned int current_inode;
	unsigned int current_queue_inode;
	unsigned int I;
	char read_buffer[BLOCK_SIZE];
	
	if(strlen(destination_file_name) == 0)
		return -1;
	
	for(I = 0; I < v->inodes_num; I++)
	{
		if(v->inodes[I].flags & VFS_FLAG_IN_USE && v->inodes[I].flags & VFS_FLAG_IS_START && strncmp(v->inodes[I].name, destination_file_name, NAME_MAX) == 0)
			return -2;
	}
	
	source_file = fopen(source_file_name, "rb");
	if(source_file == NULL)
		return -3;
	
	fseek(source_file, 0, SEEK_END);
	source_file_length = ftell(source_file);
	fseek(source_file, 0, SEEK_SET);
	
	required_inodes = vfs_required_inodes_for(source_file_length);
	nodes_queue = malloc(required_inodes * sizeof(unsigned int));
	
	current_queue_inode = 0;
	for(current_inode = 0; current_inode < v->inodes_num; current_inode++)
	{
		if((v->inodes[current_inode].flags & VFS_FLAG_IN_USE) == 0)
			nodes_queue[current_queue_inode++] = current_inode;
		if(current_queue_inode == required_inodes)
			break;
	}
	
	if(current_queue_inode < required_inodes)
	{
		free(nodes_queue);
		fclose(source_file);
		
		return -4;
	}
	
	for(I = 0; I < required_inodes; I++)
	{
		v->inodes[nodes_queue[I]].flags = VFS_FLAG_IN_USE;
		v->inodes[nodes_queue[I]].size = fread(read_buffer, 1, sizeof(read_buffer), source_file);
		
		fseek(v->F, vfs_get_block_position(v, nodes_queue[I]), SEEK_SET);
		fwrite(read_buffer, 1, v->inodes[I].size, v->F);
		if(I == 0)
		{
			v->inodes[nodes_queue[I]].flags |= VFS_FLAG_IS_START;
			strncpy(v->inodes[nodes_queue[I]].name, destination_file_name, NAME_MAX);
		}
		if(I < required_inodes - 1)
			v->inodes[nodes_queue[I]].next_node = nodes_queue[I + 1];
		else
			v->inodes[nodes_queue[I]].next_node = -1;
	}
	
	free(nodes_queue);
	fclose(source_file);
	
	return required_inodes;
}
int vfs_copy_from(const struct vfs * v, const char * source_file_name, const char * destination_file_name)
{
	FILE * destination_file;
	unsigned int I;
	unsigned int start_node;
	
	char buffer[BLOCK_SIZE];
	
	destination_file = fopen(destination_file_name, "wb");
	if(!destination_file)
		return -1;
	
	
	start_node = -1;
	for(I = 0; I < v->inodes_num; I++)
	{
		if(v->inodes[I].flags & VFS_FLAG_IN_USE && v->inodes[I].flags & VFS_FLAG_IS_START && strncmp(v->inodes[I].name, source_file_name, NAME_MAX) == 0)
		{
			start_node = I;
			break;
		}
	}
	
	if(start_node == -1)
		return -2;
	
	while(start_node != -1)
	{
		if(fread(buffer, 1, v->inodes[start_node].size, v->F) != v->inodes[start_node].size)
		{
			fclose(destination_file);
			return -3;
		}
		fwrite(buffer, 1, v->inodes[start_node].size, destination_file);
		
		start_node = v->inodes[start_node].next_node;
	}
	
	fclose(destination_file);
	
	return 1;
	
}
int vfs_delete_file(const struct vfs * v, const char * file_name)
{
	unsigned int I;
	unsigned int start_node;
	
	start_node = -1;
	for(I = 0; I < v->inodes_num; I++)
	{
		if(v->inodes[I].flags & VFS_FLAG_IN_USE && v->inodes[I].flags & VFS_FLAG_IS_START && strncmp(v->inodes[I].name, file_name, NAME_MAX) == 0)
		{
			start_node = I;
			break;
		}
	}
	
	if(start_node == -1)
		return -1;
	
	while(start_node != -1)
	{
		v->inodes[start_node].flags &= ~VFS_FLAG_IN_USE;
		start_node = v->inodes[start_node].next_node;
	}
	
	return 1;
}



unsigned int vfs_inodes_from_size(const size_t size)
{
	return (size - sizeof(struct vfs_superblock)) / (sizeof(struct vfs_inode) + BLOCK_SIZE);
}
unsigned int vfs_required_inodes_for(const size_t size)
{
	size_t size_remaining;
	unsigned int required_inodes = 0;
	size_t current_block_length;
	
	size_remaining = size;
	
	do
	{
		required_inodes++;
		
		current_block_length = size_remaining;
		if(current_block_length > BLOCK_SIZE)
			current_block_length = BLOCK_SIZE;
		
		size_remaining -= current_block_length;
	}
	while(size_remaining > 0);
	
	return required_inodes;
}
size_t vfs_get_block_position(const struct vfs * v, const size_t inode)
{
	return sizeof(struct vfs_superblock) + sizeof(struct vfs_inode) * v->inodes_num + BLOCK_SIZE * inode;
}