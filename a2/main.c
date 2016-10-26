#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

volatile sig_atomic_t STOP = 0;

void time2stop(int sig) {
	STOP = 1;
}

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
	y = (index * 10) / 100; 
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
	pthread_exit(0);
}


// Prints out the table
void print_gol(struct table_str *arg_str) {
	int i, j;
	printf("\n");
	for(i = 0; i < 100; i++) {
		for(j = 0; j < 100; j++)
			printf("%c ", arg_str->table[i][j]);
		printf("\n");
	}
	printf("\n");
}

void update_gol(struct table_str *arg_str) {
	int i, j;
	for(i = 0; i < 100; i++) 
		for(j = 0; j < 100; j++)
			arg_str->table[i][j] = arg_str->tableN[i][j];
}

void cleanOrder(struct table_str *arg_str) {
	int i;
	for(i = 0; i < 100; i++)
		arg_str->order[i] = 0;
}

void main(int argc, char **argv) {

	signal(SIGINT, time2stop);
	
	struct table_str *arg_str;
	arg_str = (struct table_str*)malloc(sizeof(struct table_str));

	int i, j, k;
	for(i = 0; i < 100; i++) {
		for(j = 0; j < 100; j++)
			scanf(" %c", &(arg_str->table[i][j]));
		arg_str->order[i] = 0;
	}

	pthread_t tids[100];
	
	// Main Loop
	while(1) {
		for(i = 0; i < 100; i++) {
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_create(&(tids[i]), &attr, thread_runner, arg_str);
		}

	
		// Wait until threads are done
		for(i = 0; i < 100; i++) 
			pthread_join(tids[i], NULL);

		update_gol(arg_str);
		system("clear");
		print_gol(arg_str);
		cleanOrder(arg_str);
		if(STOP) {
			printf("I hope you liked this Game Of Life implementation :D.\n");
			exit(EXIT_SUCCESS);
		}
	}
}
