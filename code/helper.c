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

    // random number for testing purposes
    int num = rand() % 10000 + 1;
    char randStr[6];
    memset(randStr, '\0', 6);
    sprintf(randStr, "%d", num);

    // create a copy of input string
    char* tempStr = calloc(strlen(line) + 1, sizeof(char));
    memset(tempStr, '\0', strlen(tempStr));
    strcpy(tempStr, line);
    free(line);
    line = NULL;

    char* subStr = NULL;
    char* newStr = NULL;
    while (1) {
        // find first occurence of $$
        subStr = strstr(tempStr, "$$");
        printf("subStr: [%s]\n", subStr);

        // if there is no $$ in string
        if (subStr == NULL)
            return tempStr;

        // allocate space for a new string
        newStr = calloc(strlen(tempStr) + strlen(randStr) - 2 + 1, sizeof(char));
        memset(newStr, '\0', strlen(tempStr) + strlen(randStr) - 2 + 1);

        // copy over 1st part of the string (before $$)
        int i = 0;
        while (&tempStr[i] != subStr) {
            newStr[i] = tempStr[i];
            i++;
        }

        // append the PID
        strcat(newStr, randStr);
        // append the rest of the string (after $$)
        strcat(newStr, subStr + (2 * sizeof(char))); // increment subStr ptr over 2 places

        printf("newStr: [%s]", newStr);

        // free old string
        free(tempStr);
        tempStr = newStr;
    }
}