#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <pthread.h>

struct table_str {
	char table[100][100];
	int  order[100];
	char tableN[100][100];
};

void *thread_runner(void* arg) {
	struct table_str *work = (struct table_str*)arg;
	int i = 0;
	while(i < 100) {
		if(work->order[i] == 0) {
			work->order[i] = 1;
			break;
		}
		i++;
	}
	if(i == 100) {
		fprintf(stderr, "Tried to work outside of table bound in thread.\n");
		exit(EXIT_FAILURE);
	}	
	int index = i;
	int y,x;
	y = (index * 10) / 100; // TODO: work on index > 9
	x = (index * 10) % 100;

	int xS = x;

	int ty,tx;

	// point to the right start
	y *= 10;

	for(ty = 0; ty < 10; ty++) {
		for(tx = 0; tx < 10; tx++) {
			work->tableN[y][x] = work->table[y][x];
			x++;
		}
		x = xS;
		y++;
	}
}

void main(int argc, char **argv) {
	
	struct table_str *arg_str;
	arg_str = (struct table_str*)malloc(sizeof(struct table_str));

	int i, j, k;
	for(i = 0; i < 100; i++) {
		for(j = 0; j < 100; j++)
			scanf(" %c", &(arg_str->table[i][j]));
		arg_str->order[i] = 0;
	}

	for(i = 0; i < 100; i++) {
		thread_runner((void*)arg_str);
	}

	/*
	// Prints out the table
	for(i = 0; i < 100; i++) {
		for(j = 0; j < 100; j++)
			printf("%c ", arg_str->tableN[i][j]);
		printf("\n");
	}
	*/
		
}
