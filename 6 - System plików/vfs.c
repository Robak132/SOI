#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "vfs.h"

struct FileSystem* create(const char * file_name, const size_t size) {
	FILE* F;
	
	char zero_block[128];
	
	size_t bytes_remaining;
	size_t bytes_to_write;

	struct SuperBlock sb;
	struct Node* nodes;
	struct FileSystem* v;
	unsigned int nodes_num;
	unsigned int I;
	
	F = fopen(file_name, "wb");
	if(F) {
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
		
		nodes_num = vfs_inodes_from_size(size);
		
		nodes = malloc(sizeof(struct Node) * nodes_num);
		for(I = 0; I < nodes_num; I++)
			nodes[I].flags = 0;
		
		v = malloc(sizeof(struct FileSystem));
		v->F = F;
		v->nodes_num = nodes_num;
		v->nodes = nodes;
		
		return v;
	}
	return NULL;
}
struct FileSystem* open(const char * file_name) {
	FILE * F;
	struct Node* nodes;
	struct FileSystem* v;
	
	size_t size_file;
	struct SuperBlock sb;
	unsigned int nodes_num;
	
	F = fopen(file_name, "r+b");
	if(!F)
		return NULL;
	
	fseek(F, 0, SEEK_END);
	size_file = ftell(F);
	
	if(size_file < sizeof(struct SuperBlock))
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
	
	nodes_num = vfs_inodes_from_size(size_file);
	
	nodes = malloc(sizeof(struct Node) * nodes_num);
	if(fread(nodes, sizeof(struct Node), nodes_num, F) <= 0)
	{
		fclose(F);
		free(nodes);
		return NULL;
	}
	
	v = malloc(sizeof(struct FileSystem));
	v->F = F;
	v->nodes_num = nodes_num;
	v->nodes = nodes;
	
	return v;
}

void close_file(struct FileSystem* v) {
	fseek(v->F, sizeof(struct SuperBlock), SEEK_SET);
	fwrite(v->nodes, sizeof(struct Node), v->nodes_num, v->F);
	
	fclose(v->F);
	free(v->nodes);
	free(v);
	v = NULL;
}
void unlink_file(const char* file_name) {
	unlink(file_name);
}

void dump(const struct FileSystem* v) {
	unsigned int I;
	unsigned int free_nodes;
	
	for(I = 0; I < v->nodes_num; I++)
	{
		printf("= ID: %d\n", I);
		printf("= Flags: %s%s\n", v->nodes[I].flags & FLAG_IN_USE ? "X" : "-", v->nodes[I].flags & FLAG_IS_START ? "X" : "-");
		printf("= Name: %s\n", v->nodes[I].name);
		printf("= Size: %d\n", v->nodes[I].size);
		printf("= Next: %d\n", v->nodes[I].next_node);
		printf("\n");
	}
	
	printf("== Usage log:\n");
	free_nodes = 0;
	for(I = 0; I < v->nodes_num; I++) {
		if(!(v->nodes[I].flags & FLAG_IN_USE))
			free_nodes++;	
	}
	printf("== Free nodes: %d/%d\n", free_nodes, v->nodes_num);
}
void list(const struct FileSystem* v) {
	unsigned int I;
	for(I = 0; I < v->nodes_num; I++)
	{
		if((v->nodes[I].flags & FLAG_IN_USE) && (v->nodes[I].flags & FLAG_IS_START))
		{
			printf("File: %s @%d\n", v->nodes[I].name, I);
		}
	}
}
int copyInside(const struct FileSystem* v, const char* source_file_name, const char* destination_file_name) {
	FILE * source_file;
	unsigned int* nodes_queue;

	size_t source_file_length;
	unsigned int required_inodes;
	unsigned int current_inode;
	unsigned int current_queue_inode;
	unsigned int I;
	char read_buffer[BLOCK_SIZE];
	
	if(strlen(destination_file_name) == 0)
		return -1;
	
	for(I = 0; I < v->nodes_num; I++){
		if(v->nodes[I].flags & FLAG_IN_USE && v->nodes[I].flags & FLAG_IS_START && strncmp(v->nodes[I].name, destination_file_name, NAME_MAX) == 0)
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
	for(current_inode = 0; current_inode < v->nodes_num; current_inode++) {
		if((v->nodes[current_inode].flags & FLAG_IN_USE) == 0)
			nodes_queue[current_queue_inode++] = current_inode;
		if(current_queue_inode == required_inodes)
			break;
	}
	
	if(current_queue_inode < required_inodes) {
		free(nodes_queue);
		fclose(source_file);
		
		return -4;
	}
	
	for(I = 0; I < required_inodes; I++) {
		v->nodes[nodes_queue[I]].flags = FLAG_IN_USE;
		v->nodes[nodes_queue[I]].size = fread(read_buffer, 1, sizeof(read_buffer), source_file);
		
		fseek(v->F, vfs_get_block_position(v, nodes_queue[I]), SEEK_SET);
		fwrite(read_buffer, 1, v->nodes[I].size, v->F);
		if(I == 0)
		{
			v->nodes[nodes_queue[I]].flags |= FLAG_IS_START;
			strncpy(v->nodes[nodes_queue[I]].name, destination_file_name, NAME_MAX);
		}
		if(I < required_inodes - 1)
			v->nodes[nodes_queue[I]].next_node = nodes_queue[I + 1];
		else
			v->nodes[nodes_queue[I]].next_node = -1;
	}
	
	free(nodes_queue);
	fclose(source_file);
	
	return required_inodes;
}
int copyOutside(const struct FileSystem* v, const char* source_file_name, const char* destination_file_name) {
	FILE* destination_file;
	unsigned int I;
	unsigned int start_node;
	char buffer[BLOCK_SIZE];
	
	destination_file = fopen(destination_file_name, "wb");
	if(!destination_file)
		return -1;
	
	start_node = -1;
	for(I = 0; I < v->nodes_num; I++)
	{
		if(v->nodes[I].flags & FLAG_IN_USE && v->nodes[I].flags & FLAG_IS_START && strncmp(v->nodes[I].name, source_file_name, NAME_MAX) == 0)
		{
			start_node = I;
			break;
		}
	}
	
	if(start_node == -1)
		return -2;
	
	while(start_node != -1)
	{
		if(fread(buffer, 1, v->nodes[start_node].size, v->F) != v->nodes[start_node].size)
		{
			fclose(destination_file);
			return -3;
		}
		fwrite(buffer, 1, v->nodes[start_node].size, destination_file);
		
		start_node = v->nodes[start_node].next_node;
	}
	
	fclose(destination_file);
	
	return 1;
	
}
int delete(const struct FileSystem* v, const char* file_name) {
	unsigned int I;
	unsigned int start_node = -1;

	for(I = 0; I < v->nodes_num; I++)
	{
		if(v->nodes[I].flags & FLAG_IN_USE && v->nodes[I].flags & FLAG_IS_START && strncmp(v->nodes[I].name, file_name, NAME_MAX) == 0)
		{
			start_node = I;
			break;
		}
	}
	
	if(start_node == -1)
		return -1;
	
	while(start_node != -1)
	{
		v->nodes[start_node].flags &= ~FLAG_IN_USE;
		start_node = v->nodes[start_node].next_node;
	}
	
	return 1;
}

unsigned int vfs_inodes_from_size(const size_t size)
{
	return (size - sizeof(struct SuperBlock)) / (sizeof(struct Node) + BLOCK_SIZE);
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
size_t vfs_get_block_position(const struct FileSystem* v, const size_t inode)
{
	return sizeof(struct SuperBlock) + sizeof(struct Node) * v->nodes_num + BLOCK_SIZE * inode;
}