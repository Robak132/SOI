#include <stdio.h>
#include <stdlib.h>
#include <lib.h>
#include <unistd.h>

int settime_a(int time) {
	message mess;
	mess.m1_i1 = time;
	return _syscall(MM, SETSCHUA, &mess);
}
int settime_b(int time) {
	message mess;
	mess.m1_i1 = time;
	return _syscall(MM, SETSCHUB, &mess);
}
int gettime(int group) {
	message mess;
	mess.m1_i1 = group;
	return _syscall(MM, GETTIME, &mess);
}

int main(int argc, char* argv[]) {
	int time_a = gettime(0);
	int time_b = gettime(1);
	int time_c = gettime(2);
	int new_a, new_b;

	printf("Obecne czasy:\n");
	printf("A: %d\nB: %d\nC: %d\n", time_a, time_b, time_c);
	
	new_a = atoi(argv[1]);
	printf("Zmiana A na %d\n", new_a);
	
	if (settime_a(new_a) != -2) {
		time_a = gettime(0);
		time_b = gettime(1);
		time_c = gettime(2);

		printf("Obecne czasy:\n");
		printf("A: %d\nB: %d\nC: %d\n", time_a, time_b, time_c);
	}
	else
		printf("Bledne dane dla A\n");
		
	new_b = atoi(argv[2]);
	printf("Zmiana B na %d\n", new_b);
	
	if (settime_b(new_b) != -2) {
		time_a = gettime(0);
		time_b = gettime(1);
		time_c = gettime(2);

		printf("Obecne czasy:\n");
		printf("A: %d\nB: %d\nC: %d\n", time_a, time_b, time_c);
	}
	else
		printf("Bledne dane dla B\n");
		
	
}