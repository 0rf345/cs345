#define VAR_NAME_SIZE 100
#define ARG_NUMBER 100

struct varBoard {
	char varKey[VAR_NAME_SIZE];
	char varVal[ARG_NUMBER][VAR_NAME_SIZE];
	struct varBoard* next;
};
