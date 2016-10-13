#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "helper.h"

#define TRUE 1
#define FALSE 0
#define ARG_LIMIT 1000
#define VAR_LIMIT 100

void type_prompt(void);
void read_command_parameters(char **arguments);
void tokenize(char *line, char **arguments);
int stringCounter(char **arguments);
int checkSpecial(char **arguments, int num, char *special);
void nullify(char **arguments, int num);
int countLetters(char* str);
void parseSet(struct varBoard *sentinel, char **arguments, int num);
void parseUnset(struct varBoard *sentinel, char *varKey);
void printLvars(struct varBoard *sentinel);
void parseVars(char **arguments, struct varBoard *sentinel);

void main(int argc, char *argv[]) {

	int status;
	char *arguments[ARG_LIMIT];
	char *varKeys[VAR_LIMIT];
	char **varVals[VAR_LIMIT];
	struct varBoard sentinel;
	strcpy(sentinel.varKey, "sentinel");
	sentinel.next = NULL;

	int numOfArguments;

	while(TRUE) {

		numOfArguments = 0;

		type_prompt();
		read_command_parameters(arguments);
		
		numOfArguments = stringCounter(arguments);

		int pipe_pos, out_pos, app_pos, in_pos;
		pipe_pos = out_pos = app_pos = in_pos = -1;
		pipe_pos = checkSpecial(arguments, numOfArguments, "|");
		out_pos = checkSpecial(arguments, numOfArguments, ">");
		app_pos = checkSpecial(arguments, numOfArguments, ">>");
		in_pos = checkSpecial(arguments, numOfArguments, "<");
		int lastC = countLetters(arguments[0]);
		if(numOfArguments == 1) {
			parseVars(arguments, &sentinel);
		}
		if(strcmp(arguments[0], "exit") == 0) {
			printf("Thank you for using cs345sh :)\n");
			return;
		}else if(strcmp(arguments[0], "set") == 0) {
			parseSet(&sentinel, arguments, numOfArguments);
			nullify(arguments, numOfArguments);
		}else if(strcmp(arguments[0], "unset") == 0) {
			if(arguments[1] != NULL) {
				parseUnset(&sentinel, arguments[1]);
				nullify(arguments, numOfArguments);
			}else{
				printf("What should I unset?\n");
				exit(EXIT_FAILURE);
			}
		}else if(strcmp(arguments[0], "printlvars") == 0) {
			printLvars(&sentinel);
			nullify(arguments, numOfArguments);
		}else if(out_pos != -1 || app_pos != -1 || in_pos != -1) {
			char **tempArgs;
			int max = app_pos;
			if(out_pos > max) max = out_pos;
			if(in_pos > max)max = in_pos;
			tempArgs = (char**)malloc(((sizeof(char*) * max) + 1));
			int i;
			for(i = 0; i < max; i++)
				tempArgs[i] = arguments[i];
			tempArgs[max] = NULL;

			FILE *fp;
			// Write
			if(out_pos != -1) 
				fp = fopen(arguments[out_pos + 1], "w+");
			// Append
			else if(app_pos != -1)
				fp = fopen(arguments[app_pos + 1], "a");
			// Read
			else
				fp = fopen(arguments[in_pos + 1], "r");

			int fd = fileno(fp);
			pid_t child_pid = fork();
			if(child_pid == 0) {
				if(in_pos != -1) dup2(fd, 0);
				else dup2(fd, 1);
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

// set var
void parseSet(struct varBoard *sentinel, char **arguments, int num) {
	char *var = (char*)malloc(sizeof(char) * 100);
	int i = 0;
	while(1) {
		var[i] = arguments[1][i];
		i++;
		if(arguments[1][i] == '=') 	
			break;
	}
	var[i] = '\0';

	char *newArgs[num];
	// jump "
	i += 2;
	int j = 0;
	newArgs[0] = (char*)malloc(sizeof(char) * 100);
	while(1) {
		if(arguments[1][i] == '\"') {
			newArgs[0][j] = '\0';
			break;
		}
		newArgs[0][j] = arguments[1][i];
		j++;
		i++;
		if(newArgs[0][j-1] == '\0') break;
	}
	i = 2;
	j = 1;
	while(i < (num - 1)) {
		newArgs[j] = (char*)malloc(sizeof(char) * 100);
		strcpy(newArgs[j], arguments[i]);
		j++;
		i++;
	}
	// last argument
	newArgs[j] = (char *)malloc(sizeof(char) * 100);
	int k = 0;
	while(1) {
		newArgs[j][k] = arguments[num - 1][k];
		k++;
		if(arguments[num - 1][k] == '\"') {
			newArgs[j][k] = '\0';
			break;
		}
	}
	newArgs[num - 1] = NULL;

	struct varBoard *traverse = sentinel;
	int same = 0;
	while(1) {
		if(traverse->next == NULL) break;
		if(strcmp(traverse->next->varKey, var) == 0) break;
		traverse = traverse->next;
	}
	// TOD  next for sameO
	traverse->next = (struct varBoard*)malloc(sizeof(struct varBoard));
	strcpy(traverse->next->varKey, var);
	for(i = 0; i < num - 1; i++)
		strcpy(traverse->next->varVal[i], newArgs[i]);
	strcpy(traverse->next->varVal[num - 1], "\0");
}

void parseUnset(struct varBoard *sentinel, char *varKey) {
	struct varBoard *traverse = sentinel;
	while(1) {
		if(traverse->next == NULL) break;
		if(strcmp(traverse -> next->varKey, varKey) == 0) {
			traverse->next = traverse->next->next;
			break;
		}
		traverse = traverse->next;
	}
}

void printLvars(struct varBoard *sentinel) {
	struct varBoard *traverse = sentinel;
	traverse = traverse->next;
	while(traverse != NULL) {
		printf("%s=", traverse->varKey);
	
		// there is some inexcpicable ghost when I am trying to do this
		// in a loop so I limit it to 20 args which is more than
		// reasonable amount of arguments
		/*	
		if(traverse->varVal[0]) printf("%s ", traverse->varVal[0]);
		if(traverse->varVal[1]) printf("%s ", traverse->varVal[1]);
		if(traverse->varVal[2]) printf("%s ", traverse->varVal[2]);
		if(traverse->varVal[3]) printf("%s ", traverse->varVal[3]);
		if(traverse->varVal[4]) printf("%s ", traverse->varVal[4]);
		if(traverse->varVal[5]) printf("%s ", traverse->varVal[5]);
		if(traverse->varVal[6]) printf("%s ", traverse->varVal[6]);
		if(traverse->varVal[7]) printf("%s ", traverse->varVal[7]);
		if(traverse->varVal[8]) printf("%s ", traverse->varVal[8]);
		if(traverse->varVal[9]) printf("%s ", traverse->varVal[9]);
		if(traverse->varVal[10]) printf("%s ", traverse->varVal[10]);
		if(traverse->varVal[11]) printf("%s ", traverse->varVal[11]);
		if(traverse->varVal[12]) printf("%s ", traverse->varVal[12]);
		if(traverse->varVal[13]) printf("%s ", traverse->varVal[13]);
		if(traverse->varVal[14]) printf("%s ", traverse->varVal[14]);
		if(traverse->varVal[15]) printf("%s ", traverse->varVal[15]);
		if(traverse->varVal[16]) printf("%s ", traverse->varVal[16]);
		if(traverse->varVal[17]) printf("%s ", traverse->varVal[17]);
		if(traverse->varVal[18]) printf("%s ", traverse->varVal[18]);
		if(traverse->varVal[19]) printf("%s ", traverse->varVal[19]);
		if(traverse->varVal[20]) printf("%s ", traverse->varVal[20]);
		if(traverse->varVal[21]) printf("%s ", traverse->varVal[21]);
		*/
		int i;
		for(i = 0; traverse->varVal[i][0] != '\0'; i++) {
			printf("%s ", traverse->varVal[i]);
		}
		printf("\n");
		traverse = traverse->next;
	}
}

void parseVars(char **arguments, struct varBoard *sentinel) {
	struct varBoard *traverse = sentinel;
	int found = 0;
	while(traverse = traverse->next) {
		if(strcmp(traverse->varKey, arguments[0]) == 0) {
			found = 1;
			break;
		}
	}
	if(found == 0) return;
	int i = 0;
	while(traverse->varVal[i][0] != '\0') {
		arguments[i] = (char*)malloc(sizeof(char) * 100);
		strcpy(arguments[i], traverse->varVal[i]);
		i++;
	}
	return;
}
