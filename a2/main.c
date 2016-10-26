#include <stdio.h>
#include <string.h>
//#include <pthread.h>


void main(int argc, char **argv) {
	char table[100][100];

	int i, j;
	for(i = 0; i < 100; i++)
		for(j = 0; j < 100; j++)
			scanf(" %c", &(table[i][j]));
	
	/*
	// Prints out the table
	for(i = 0; i < 100; i++) {
		for(j = 0; j < 100; j++)
			printf("%c ", table[i][j]);
		printf("\n");
	}
	*/
}
