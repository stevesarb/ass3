#include "prototypes.h"

void cd(char* line, int lineSize) {
    // if (DEBUGMODE)
	//     printf("Path: [%s]\n", &line[3]);

    // cd to home directory
    if (lineSize == 3) {
        if (chdir(getenv("HOME")) != 0)
            perror("cd to home directory failed\n");
    }
    else {
        char* path = calloc(strlen(line) - 3, sizeof(char));
        strncpy(path, &line[3], lineSize - 4); // eliminates newline character
        if (chdir(path) != 0)
            perror("cd failed");
        
    }

    // if (1) {
    // 	int num = rand() % 100 + 1;
    // 	char rand_num_str[5];
    // 	char fName[15];
    // 	memset(rand_num_str, '\0', 5);
    // 	sprintf(rand_num_str, "%d", num);
    // 	memset(fName, '\0', 15);
    // 	strcat(fName, rand_num_str);
    // 	strcat(fName, ".smallsh");
    // 	int fDesc = open(fName, O_CREAT, 0640);
    // }
}

char* search_for_dollars(char* line, int lineSize) {
    printf("line: [%s]\nlineSize: [%d]\n", line, lineSize);

    // char* str = calloc(1000, sizeof(char));
    // memset(str, '\0', 1000);

    int* idxArr = NULL;

    for (int i = 0; i < lineSize - 1; i++) {
        if ((line[i] == '$') && (line[i + 1] == '$')) {
            // expand $$
            // for (int j = 0; j < i; j++) {
            //     str[j] = line[j];
            // }

            // add the index of $$ to idxArr
            i++;
        }
    }
}