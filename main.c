#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TRUE 1

void type_prompt(void);

void read_command_parameters();

void main(int argc, char *argv[]) {

	int status;
	char *arguements[20];

	while(TRUE) {

		type_prompt();
		read_command_parameters();

		pid_t child_pid = fork();

		if(child_pid == 0) {
			//child
			4 + 5;	
			printf("Inside the child process\n");	
			//execv("/bin/ls", arguements); THIS IS AN EXAMPLE
			//arguemetns[0] = program;
			//execv("/bin/ls", mpla);
		}else{
			//parent code
			printf("Inside the parent process\n");
			wait(&status);
		}
		
	}

}

void type_prompt(void) {
	struct passwd *pws;
	pws = getpwuid(geteuid());
	if(pws != NULL) {
		fprintf(stdout, "%s@cs345sh%s$ ", pws->pw_name, getcwd(NULL, 1000));
		return;
	}else{
		fprintf(stderr, "Error finding the current user");
		exit(-1);
	}
}

void read_command_parameters() {
	char *line;
	line = NULL;
	size_t len = 0;

	getline(&line, &len, stdin);

}
