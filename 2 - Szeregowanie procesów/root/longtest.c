#include <stdio.h>
#include <stdlib.h>
#include <lib.h>
#include <unistd.h>
#include <time.h>

void setgroup(int pid, int group) {
	message mess;
	mess.m1_i1 = pid;
	mess.m1_i2 = group;
	_syscall(MM, SETGROUP, &mess);
}

int main(int argc, char* argv[]) {
	int pid = getpid();
	int group = argv[1][0] - '0';
	int i, k;
	double t1, t2, total;

	setgroup(pid, group);
	
	t1 = time(0);
	t2 = clock();
	for (k=0;k<20;k++) {
		for(i=1;i<2000000000;i++);
	}
	t1 = time(0) - t1;
	t2 = (double)(clock() - t2)/CLOCKS_PER_SEC;
	total = t1-t2;
	
	printf("Proces zakonczony, gr. %d, czas bezwzgl %f, czas wzgl %f, czas ocz %f\n", group, t1, t2, total);
	return 0;
}