#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "t_params.h"
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#define set_total_c_time_sys(ipid, time) syscall(342, ipid, time)
#define get_total_c_time_sys(ipid, param) syscall(343, ipid, param)


void main(void) {
	printf("Trapping to kernel level\n");
	pid_t pid1, pid2, pid3, pid4, pid5;
	struct t_params check[5];
	
	// Child process test
	pid1 = fork();
	pid2 = fork();
	pid3 = fork();
	pid4 = fork();
	pid5 = fork();
	
	// child is playing with toys
	if(pid1 == 0 || pid2 == 0 || pid3 == 0 || pid4 == 0 || pid5 == 0) {
		unsigned int i;
		unsigned int k;
		k = 0;
		for(i = 0;; i++)
			k++;
	}else{
		set_total_c_time_sys(getpid(), 831);
		set_total_c_time_sys(pid1, 151);
		set_total_c_time_sys(pid2, 152);
		set_total_c_time_sys(pid3, 153);
		set_total_c_time_sys(pid4, 154);
		set_total_c_time_sys(pid5, 155);
		
		get_total_c_time_sys(getpid(), &check);
		get_total_c_time_sys(pid1, &check[0]);		
		get_total_c_time_sys(pid2, &check[1]);
		get_total_c_time_sys(pid3, &check[2]);
		get_total_c_time_sys(pid4, &check[3]);
		get_total_c_time_sys(pid5, &check[4]);
	
		kill(pid1, SIGKILL);
		kill(pid2, SIGKILL);
		kill(pid3, SIGKILL);
		kill(pid4, SIGKILL);
		kill(pid5, SIGKILL);
	}
	
	printf("Back to user level\n");
	
	printf("PID1: %d, rem time: %d, infFlag: %d\n", pid1, check[0].remaining_time, check[0].infinite);	
	printf("PID2: %d, rem time: %d, infFlag: %d\n", pid2, check[1].remaining_time, check[1].infinite);	
	printf("PID3: %d, rem time: %d, infFlag: %d\n", pid3, check[2].remaining_time, check[2].infinite);	
	printf("PID4: %d, rem time: %d, infFlag: %d\n", pid4, check[3].remaining_time, check[3].infinite);	
	printf("PID5: %d, rem time: %d, infFlag: %d\n", pid5, check[4].remaining_time, check[4].infinite);	

	exit(EXIT_SUCCESS);
}
