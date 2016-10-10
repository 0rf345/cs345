#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TRUE 1

void type_prompt(void);

void read_command_parameters(char **arguments);

void tokenize(char *line, char **arguments);

int stringCounter(char **arguments);

void main(int argc, char *argv[]) {

	int status;
	char *arguments[1000];
	int numOfArguments;

	while(TRUE) {

		numOfArguments = 0;

		type_prompt();
		read_command_parameters(arguments);
		
		numOfArguments = stringCounter(arguments);

		pid_t child_pid = fork();

		if(child_pid == 0) {
			//child
			4 + 5;	
			printf("Inside the child process\n");	
			//execv("/bin/ls", arguments); THIS IS AN EXAMPLE
			//argumetns[0] = program;
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

/*
	line will be used to store what 
	the user inputs in our program
*/
void read_command_parameters(char **arguments) {
	char *line;
	size_t len = 0;

	getline(&line, &len, stdin);
	tokenize(line, arguments);
}

// Splits line into strings by usins ' ' as separators
void tokenize(char *line, char **arguments) {
	int i = 0;
	arguments[i] = strtok(line, " ");	
	while(arguments[i] != NULL) {
		i++;
		arguments[i] = strtok(NULL, " ");
	}
}

// Returns how many strings are contained in arguments
int stringCounter(char **arguments) {
	int i = 0;
	while(arguments[i] != NULL)
		i++;
	return i;
}
