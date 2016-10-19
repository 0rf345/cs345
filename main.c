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
void nullify(char **arguments);
int countLetters(char* str);
void parseSet(struct varBoard *sentinel, char **arguments, int num);
void parseUnset(struct varBoard *sentinel, char *varKey);
void printLvars(struct varBoard *sentinel);
void parseVars(char **arguments, struct varBoard *sentinel);
void printVar(char *var, struct varBoard *sentinel);

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
		nullify(arguments);
		read_command_parameters(arguments);
		
		numOfArguments = stringCounter(arguments);
		
		if(numOfArguments == 1 && arguments[0][0] == '$') {
			int i = 1;
			while(arguments[0][i] != '\0') {
				arguments[0][i - 1] = arguments[0][i];
				i++;
			}
			arguments[0][i - 1] = '\0';
			parseVars(arguments, &sentinel);
		}

		int pipe_pos, out_pos, app_pos, in_pos;
		pipe_pos = out_pos = app_pos = in_pos = -1;
		pipe_pos = checkSpecial(arguments, numOfArguments, "|");
		out_pos = checkSpecial(arguments, numOfArguments, ">");
		app_pos = checkSpecial(arguments, numOfArguments, ">>");
		in_pos = checkSpecial(arguments, numOfArguments, "<");
		int lastC = countLetters(arguments[0]);
		if(strcmp(arguments[0], "exit") == 0) {
			printf("Thank you for using cs345sh :)\n");
			exit(EXIT_SUCCESS);
		}else if(strcmp(arguments[0], "set") == 0) {
			parseSet(&sentinel, arguments, numOfArguments);
		}else if(strcmp(arguments[0], "unset") == 0) {
			if(arguments[1] != NULL) {
				parseUnset(&sentinel, arguments[1]);
			}else{
				printf("What should I unset?\n");
				exit(EXIT_FAILURE);
			}
		}else if(numOfArguments == 2 && (strcmp((char*)(arguments[0]), "echo") == 0) &&
			arguments[1][0] == '$') {
			int i = 1;
			while(arguments[1][i] != '\0') {
				arguments[1][i - 1] = arguments[1][i];
				i++;
			}
			arguments[1][i - 1] = '\0';
			printVar(arguments[1], &sentinel);
		}else if(strcmp(arguments[0], "printlvars") == 0) {
			printLvars(&sentinel);
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
			}
		}else if(strcmp(arguments[0], "cd") == 0) {
			chdir(arguments[1]);
		}else if(pipe_pos == -1) {
			pid_t child_pid = fork();

			if(child_pid == 0) {
				execvp(arguments[0], arguments);
				printf("Could not find command %s\n", arguments[0]);
			}else{
				wait(&status);
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
void nullify(char **arguments) {
	int i;
	for(i = 0; i < 1000; i++)
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
		char c = arguments[1][i]; 
		if(c == '@' || c == '!' || c == '#' || c == '$' || c == '%' ||
		c == '&' || c == '\'' || c == '\"' || c == '(' || c == ')' ||
		c == '*' || c == '+' || c == ';' || c == '<' || c == '>' ||
		c == ':' || c == '-' || c == '~' || c == '^' || c == '|' ||
		c == '[' || c == ']' || c == '{' || c == '}' || c == '\\'|| 
		c == '/' || c == '.' || c == ','){
			printf("Char %c found in var name.\n", c);
			printf("Please only use a-z, A-Z, 0-9 and '_' as var name.\n");
			return;
		}
		else {
		var[i] = arguments[1][i];
		i++;
		if(arguments[1][i] == '=') 	
			break;
		}
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
	struct varBoard *temp;
	while(1) {
		if(traverse->next == NULL) break;
		if(strcmp(traverse->next->varKey, var) == 0) {
			same = 1;
			temp = traverse->next->next;
			break;
		}
		traverse = traverse->next;
	}
	// TOD  next for sameO
	traverse->next = (struct varBoard*)malloc(sizeof(struct varBoard));
	strcpy(traverse->next->varKey, var);
	for(i = 0; i < num - 1; i++)
		strcpy((char*)(traverse->next->varVal[i]), newArgs[i]);
	strcpy((char*)traverse->next->varVal[num - 1], "\0");
	if(same)traverse->next->next = temp;
}

void parseUnset(struct varBoard *sentinel, char *varKey) {
	struct varBoard *traverse = sentinel;
	while(1) {
		if(traverse->next == NULL) break;
		if(strcmp(traverse -> next->varKey, varKey) == 0) {
			struct varBoard *temp = traverse->next;
			traverse->next = traverse->next->next;
			free(temp);
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
	
		int i;
		for(i = 0; traverse->varVal[i][0] != '\0'; i++) {
			printf("%s ", (char*)(traverse->varVal[i]));
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
		strcpy(arguments[i], (char*)(traverse->varVal[i]));
		i++;
	}
	return;
}

void printVar(char *var, struct varBoard *sentinel) {
	struct varBoard *traverse = sentinel;
	while(traverse = traverse->next) {
		if(strcmp(var, traverse->varKey) == 0) {
			printf("%s=", traverse->varKey);
			int i = 0;
			while(traverse->varVal[i][0] != '\0') {
				printf("%s ", traverse->varVal[i]);
				i++;
			}
			printf("\n");
			return;
		}
	}
	printf("Var \"%s\" not found\n", var);
}
