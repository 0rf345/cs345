#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0

void type_prompt(void);
void read_command_parameters(char **arguments);
void tokenize(char *line, char **arguments);
int stringCounter(char **arguments);
int checkSpecial(char **arguments, int num, char *special);
void nullify(char **arguments, int num);
int countLetters(char* str);

void main(int argc, char *argv[]) {

	int status;
	char *arguments[1000];
	int numOfArguments;

	while(TRUE) {

		numOfArguments = 0;

		type_prompt();
		read_command_parameters(arguments);
		
		numOfArguments = stringCounter(arguments);

		int pipe_pos, out_pos, app_pos, in_pos;
		pipe_pos = -1;
		out_pos = -1;
		app_pos = -1;
		in_pos = -1;
		pipe_pos = checkSpecial(arguments, numOfArguments, "|");
		out_pos = checkSpecial(arguments, numOfArguments, ">");
		app_pos = checkSpecial(arguments, numOfArguments, ">>");
		in_pos = checkSpecial(arguments, numOfArguments, "<");
		int lastC = countLetters(arguments[0]);
		if(strcmp(arguments[0], "exit") == 0) {
			printf("Thank you for using cs345sh :)\n");
			return;
		}else if(in_pos != -1) {
			char *tempArgs[in_pos + 1];
			int i;
			for(i = 0; i < in_pos; i++)
				tempArgs[i] = arguments[i];
			tempArgs[in_pos] = NULL;
			FILE *fp;
			fp = fopen (arguments[in_pos + 1], "r");
			int fd = fileno(fp);

			pid_t child_pid = fork();
			if(child_pid == 0) {
				dup2(fd, 0);
				execvp(tempArgs[0], tempArgs);
			}else{
				wait(&status);
				fclose(fp);
				nullify(arguments, numOfArguments);
			}
		}else if(out_pos != -1 || app_pos != -1) {
			char **tempArgs;
			int max = app_pos;
			if(out_pos > app_pos) max = out_pos;
			tempArgs = (char**)malloc((sizeof(char*) * max + 1));
			int i;
			
				for(i = 0; i < out_pos; i++)
					tempArgs[i] = arguments[i];
			
				tempArgs[out_pos] = NULL;

			FILE *fp;
			// Write
			if(strcmp(arguments[out_pos], ">") == 0) 
				fp = fopen(arguments[out_pos + 1], "w+");
			// Append
			else
				fp = fopen(arguments[app_pos + 1], "a");
			for(i = 0; i < max; i++)
				tempArgs[i] = arguments[i];
			tempArgs[max] = NULL;

			int fd = fileno(fp);
			pid_t child_pid = fork();
			if(child_pid == 0) {
				dup2(fd, 1);
				execvp(tempArgs[0], tempArgs);
			}else{
				wait(&status);
				fclose(fp);
				nullify(arguments, numOfArguments);
			}
		}else if(arguments[0][lastC - 1] == '&') {
			arguments[0][lastC - 1] = '\0';
			pid_t child_pid = fork();
			if(child_pid == 0) {
				setpgid(0, 0);
				execvp(arguments[0], arguments);
				printf("Could not find command %s\n", arguments[0]);
			}else{
				printf("Process %s with pid %d ", arguments[0], child_pid);
				printf("is now running in the background\n");
				nullify(arguments, numOfArguments);
			}
		}else if(strcmp(arguments[0], "cd") == 0) {
			chdir(arguments[1]);
			nullify(arguments, numOfArguments);
		}else if(pipe_pos == -1) {
			pid_t child_pid = fork();

			if(child_pid == 0) {
				execvp(arguments[0], arguments);
				printf("Could not find command %s\n", arguments[0]);
			}else{
				wait(&status);
				nullify(arguments, numOfArguments);				
			}
		// PIPE detected
		}else{
			pid_t child_pid = fork();
			if(child_pid == 0) {
				int pipefd[2];
				pipe(pipefd);
				pid_t nested_pid = fork();
	
				if(nested_pid == 0) {
					dup2(pipefd[0], 0);
					close(pipefd[1]);
					execvp(arguments[pipe_pos + 1], &arguments[pipe_pos + 1]);
				}else{
					dup2(pipefd[1], 1);
					close(pipefd[0]);
					char *tempArgs[pipe_pos + 1];
					int i;
					for(i = 0; i < pipe_pos; i++)
						tempArgs[i] = arguments[i];
					tempArgs[pipe_pos] = NULL;
					execvp(tempArgs[0], tempArgs);
				}	
			}else{
				wait(&status);
				nullify(arguments, numOfArguments);
			}
		}	
	}

}

/*
	Checks if there is '|' in the arguments
	returns its position if found
	returns -1 if NOT found
*/
int checkSpecial(char **arguments, int num, char *special) {
	int i;
	for(i = 0; i < num; i++) {
		if(strcmp(arguments[i], special) == 0)
			return i;
	}
	return -1;
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

// nullifies arguments array
void nullify(char **arguments, int num) {
	int i;
	for(i = 0; i < num; i++)
		arguments[i] = NULL;
}

int countLetters(char *str) {
	int i;
	i = 0;
	while(1) {
		if(str[i] == '\0') return i;
		i++;
	}
}
