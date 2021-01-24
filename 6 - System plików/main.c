#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"


int main(int ArgC, char ** ArgV)
{
	char * vfs_name;
	char * command;
	struct FileSystem * v;
	
	if(ArgC < 3) {
		printf("%s <vfs name> command (...)\n", ArgV[0]);
		printf(
			"Availible commands: \n"
			"create\n"
			"dump\n"
			"list\n"
			"push\n"
			"pull\n"
			"remove\n"
			"delete\n"
		);
		return 1;
	}
	
	vfs_name = ArgV[1];
	command = ArgV[2];
	
	if(strcmp("create", command) == 0) {
		if(ArgC == 4) {
			size_t size = atoi(ArgV[3]);
			v = create(vfs_name, size);
			if(!v) {
				printf("Error!\n");
				return 2;
			}
			close_file(v);
		}
		else
			printf("Error!\n");
	}
	else if(strcmp("dump", command) == 0) {
		if(ArgC == 3) {
			v = open(vfs_name);
			if(!v) {
				printf("Error!\n");
				return 2;
			}
			dump(v);
			close_file(v);
		}
		else
			printf("Error!\n");
	}
	else if(strcmp("list", command) == 0) {
		if(ArgC == 3) {
			v = open(vfs_name);
			if(!v) {
				printf("Error!\n");
				return 2;
			}
			list(v);
			close_file(v);
		}
		else
			printf("Error!\n");
	}
	else if(strcmp("push", command) == 0) {
		if(ArgC == 5) {
			v = open(vfs_name);
			if(!v) {
				printf("Error!\n");
				return 2;
			}
			printf("Result: %d\n", copyInside(v, ArgV[3], ArgV[4]));
			close_file(v);
		}
		else
			printf("Error!\n");
	}
	else if(strcmp("pull", command) == 0) {
		if(ArgC == 5) {
			v = open(vfs_name);
			if(!v) {
				printf("Error!\n");
				return 2;
			}
			printf("Result: %d\n", copyOutside(v, ArgV[3], ArgV[4]));
			close_file(v);
		}
		else
			printf("Error!\n");
	}
	else if(strcmp("remove", command) == 0) {
		if(ArgC == 4) {
			v = open(vfs_name);
			if(!v) {
				printf("Error!\n");
				return 2;
			}
			printf("Result: %d\n", delete(v, ArgV[3]));
			close_file(v);
		}
		else
			printf("Error!\n");
	}
	else if(strcmp("delete", command) == 0) {
		if(ArgC == 3) {
			unlink_file(vfs_name);
		}
		else
			printf("Error!\n");
	}

	return 0;
}