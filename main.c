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

void unify(char *dest, char *src);

void main(int argc, char *argv[]) {

	int status;
	char *arguments[1000];
	int numOfArguments;
	char *path1, *path2;

	path1 = (char*)malloc(sizeof(char) * 1000);
	path2 = (char*)malloc(sizeof(char) * 1000);

	strcpy(path1, "/bin/");
//	strcpy(path2, "/usr/bin/");

	while(TRUE) {

		numOfArguments = 0;

		type_prompt();
		read_command_parameters(arguments);
		
		numOfArguments = stringCounter(arguments);
	
		if(strcmp(arguments[0], "exit") == 0) {
			printf("Thank you for using cs345sh :)\n");
			return;
		}

		pid_t child_pid = fork();

		strcpy(&path1[5], arguments[0]);

		strcpy(arguments[0], path1);
		if(child_pid == 0) {
			printf("Inside the child process\n");	
			execv(path1, arguments);
			printf("Something failed with execv\n");
		}else{
			printf("Inside the parent process\n");
			wait(&status);
			int h = 0;
			while(1) {
				if(arguments[h] == NULL) break;
				arguments[h] = NULL;
				h++;
			}
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
	int i;
	for(i = 0;;i++) {
		if(line[i] == '\n') {
			line[i] = '\0';
			break;
		}
	}
	tokenize(line, arguments);
}

// Splits line into strings by usins ' ' as separators
void tokenize(char *line, char **arguments) {
	int i;
	i = 0;
	int j, k;
	j = k = 0;
	arguments[0] = (char*)malloc(sizeof(char) * 200);
	while(TRUE) {
		if(line[i] == '\0') {
			arguments[j][k] = '\0';
			break;
		}else if(line[i] == ' ') {
			arguments[j][k] = '\0';
			k = 0;
			j++;
			arguments[j] = (char*)malloc(sizeof(char) * 200);
		}else{
			arguments[j][k] = line[i];
			k++;
		}
		i++;	
	}
}

// Returns how many strings are contained in arguments
int stringCounter(char **arguments) {
	int i = 0;
	while(arguments[i] != NULL)
		i++;
	return i;
}

void unNull(char* str) {
	int i = 0;
	while(str[i] != '\0') i++;
	str[i] = str[i + 1];
}
