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
	struct t_params *check;
	// wrong pid	
	if(set_total_c_time_sys(getpid() - 1, 831) != 0) {
		fprintf(stderr, "There was an error with set of PID: %d\n", getpid() - 1);
	}else if(get_total_c_time_sys(getpid(), &check) != 0) {
		fprintf(stderr, "There was an error with get of PID: %d\n", getpid());
	}
	check = NULL;
	// null t_param argument
	if(set_total_c_time_sys(-1, 831) != 0) {
		fprintf(stderr, "There was an error with set of PID: %d\n", getpid());
	}else if(get_total_c_time_sys(getpid(), check) != 0) {
		fprintf(stderr, "There was an error with get of PID: %d\n", getpid());
	}
	
	printf("Back to user level\n");
	
	exit(EXIT_FAILURE);
}
