#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"


int main(int ArgC, char ** ArgV)
{
	char * vfs_name;
	char * command;
	struct FileSystem * v;
	
	if(ArgC < 3)
	{
		printf("%s <vfs name> command (...)\n", ArgV[0]);
		printf(
			"Availible commands: \n"
			"- create <size in bytes>\n"
			"- dump\n"
			"- list\n"
			"- push <source file name> <destination file name>\n"
			"- pull <source file name> <destination file name>\n"
			"- remove <file name>\n"
			"- delete\n"
		);
		return 1;
	}
	
	vfs_name = ArgV[1];
	command = ArgV[2];
	
	if(strcmp("create", command) == 0)
	{
		if(ArgC == 4)
		{
			size_t size = atoi(ArgV[3]);
			v = create(vfs_name, size);
			if(!v)
			{
				printf("Nie udalo sie utworzyc dysku wirtualnego!\n");
				return 2;
			}
			close_file(v);
		}
		else
			printf("%s <vfs name> create <size in bytes>\n", ArgV[0]);
	}
	else if(strcmp("dump", command) == 0)
	{
		if(ArgC == 3)
		{
			v = open(vfs_name);
			if(!v)
			{
				printf("Nie udalo sie otworzyc dysku wirtualnego!\n");
				return 2;
			}
			
			dump(v);
			
			close_file(v);
		}
		else
			printf("%s <vfs name> dump\n", ArgV[0]);
	}
	else if(strcmp("list", command) == 0)
	{
		if(ArgC == 3)
		{
			v = open(vfs_name);
			if(!v)
			{
				printf("Nie udalo sie otworzyc dysku wirtualnego!\n");
				return 2;
			}
			
			list(v);
			
			close_file(v);
		}
		else
			printf("%s <vfs name> list\n", ArgV[0]);
	}
	else if(strcmp("push", command) == 0)
	{
		if(ArgC == 5)
		{
			v = open(vfs_name);
			if(!v)
			{
				printf("Nie udalo sie otworzyc dysku wirtualnego!\n");
				return 2;
			}
			
			printf("Wysylanie pliku, wynik: %d\n", copyInside(v, ArgV[3], ArgV[4]));
			
			close_file(v);
		}
		else
			printf("%s <vfs name> push <source file name> <destination file name>\n", ArgV[0]);
	}
	else if(strcmp("pull", command) == 0)
	{
		if(ArgC == 5)
		{
			v = open(vfs_name);
			if(!v)
			{
				printf("Nie udalo sie otworzyc dysku wirtualnego!\n");
				return 2;
			}
			
			printf("Pobieranie pliku, wynik: %d\n", copyOutside(v, ArgV[3], ArgV[4]));
			
			close_file(v);
		}
		else
			printf("%s <vfs name> pull <source file name> <destination file name>\n", ArgV[0]);
	}
	else if(strcmp("remove", command) == 0)
	{
		if(ArgC == 4)
		{
			v = open(vfs_name);
			if(!v)
			{
				printf("Nie udalo sie otworzyc dysku wirtualnego!\n");
				return 2;
			}
			
			delete(v, ArgV[3]);
			
			close_file(v);
		}
		else
			printf("%s <vfs name> pull <source file name> <destination file name>\n", ArgV[0]);
	}
	else if(strcmp("delete", command) == 0)
	{
		if(ArgC == 3)
		{
			unlink_file(vfs_name);
		}
		else
			printf("%s <vfs name> delete\n", ArgV[0]);
	}
	else
	{
		printf("%s invalid command `%s`\n", ArgV[0], command);
		return 1;
	}
		
	return 0;
}