#include <stdio.h>
#include <stdlib.h>
#include <lib.h>
#include <unistd.h>
#include <limits.h>

void setgroup(int pid, int group) {
	message mess;
	mess.m1_i1 = pid;
	mess.m1_i2 = group;
	_syscall(MM, SETGROUP, &mess);
}
int getgroup(int pid) {
	message mess;
	mess.m1_i1 = pid;
	return _syscall(MM, GETGROUP, &mess);
}

int main(int argc, char* argv[]) {
	pid_t pid = getpid();
	int group;

	group = getgroup(pid);

	printf("PID: %d\n", pid);
	printf("Obecna grupa: %d\n", group);
	setgroup(pid, atoi(argv[1]));

	group = getgroup(pid);
	printf("Nowa groupa: %d\n", group);
}