#pragma once

#include <stdio.h>

#define NAME_MAX 32
#define BLOCK_SIZE 2048
#define FLAG_IN_USE (1 << 0)
#define FLAG_IS_START (1 << 1)

struct FileSystem
{
	FILE* F;
	unsigned int nodes_num;
	struct Node* nodes;
};
struct SuperBlock {
	size_t size;
};
struct Node {
	unsigned int flags;
	char name[NAME_MAX];
	unsigned int size;
	unsigned int next_node;
};

struct FileSystem* create(const char * file_name, const size_t size);
struct FileSystem* open(const char * file_name);
void close_file(struct FileSystem * v);
void unlink_file(const char * file_name);

void dump(const struct FileSystem * v);
void list(const struct FileSystem * v);
int copyInside(const struct FileSystem * v, const char * source_file_name, const char * destination_file_name);
int copyOutside(const struct FileSystem * v, const char * source_file_name, const char * destination_file_name);
int delete(const struct FileSystem * v, const char * file_name);

unsigned int findRequiredNodes(const size_t size);