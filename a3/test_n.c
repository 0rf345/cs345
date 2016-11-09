#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "t_params.h"
#include <stdlib.h>
#include <sys/types.h>

#define set_total_c_time_sys(ipid, time) syscall(342, ipid, time)
#define get_total_c_time_sys(ipid, param) syscall(343, ipid, param)


void main(void) {
	printf("Trapping to kernel level\n");
	struct t_params check;
	
	if(set_total_c_time_sys(getpid(), 831) != 0) {
		fprintf(stderr, "There was an error with set of PID: %d\n", getpid());
		exit(EXIT_FAILURE);
	}else if(get_total_c_time_sys(getpid(), &check) != 0) {
		fprintf(stderr, "There was an error with get of PID: %d\n", getpid());
		exit(EXIT_FAILURE);
	}
	
	
	printf("Remaining time is: %d\n", check.remaining_time);
	printf("Infinite flag is:  %d\n", check.infinite);
	printf("Back to user level\n");
	
	exit(EXIT_SUCCESS);
}
