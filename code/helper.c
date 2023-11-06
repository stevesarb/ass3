#include "prototypes.h"

void cd(char* line, int lineSize) {
    // if (DEBUGMODE)
	//     printf("Path: [%s]\n", &line[3]);
    char* path = NULL;

    // cd attempt in the background
    if (line[strlen(line) - 2] == '&') {
        // replace the space before the & with a '\n' and then a null terminator
        line[strlen(line) - 3] = '\n';
        line[strlen(line) - 2] = '\0';
        lineSize -= 2;
    }

    // cd to home directory
    if (lineSize == 3) {
        if (chdir(getenv("HOME")) != 0)
            perror("cd to home directory failed\n");
    }

    // cd with absolute path from home directory
    else if ((lineSize >= 5) && (line[3] == '~')) {
        char* homePath = getenv("HOME");
        // allocate space for string
        path = calloc(strlen(homePath) + strlen(line) - 3, sizeof(char));
        memset(path, '\0', strlen(homePath) + strlen(line) - 3);
        // cat homePath to path
        strcat(path, homePath);
        // replace '\n' with '\0'
        line[strlen(line) - 1] = '\0';
        // cat the remainder of the input string (cd path after ~) to path
        strcat(path, &line[4]);
        // cd
        if (chdir(path) != 0) {
            perror("cd ~ failed\n");
            fflush(stdout);
        }
    }
    else {
        path = calloc(strlen(line) - 3, sizeof(char));
        memset(path, '\0', strlen(path));
        strncpy(path, &line[3], lineSize - 4); // eliminates newline character
        if (chdir(path) != 0) {
            perror("cd failed\n");
            fflush(stdout); 
        }
    }

    if (1) {
    	int num = rand() % 100 + 1;
    	char rand_num_str[50];
    	char fName[60];
    	memset(rand_num_str, '\0', 50);
    	sprintf(rand_num_str, "%d", num);
    	memset(fName, '\0', 60);
    	strcat(fName, rand_num_str);
    	strcat(fName, ".smallsh");
    	int fDesc = open(fName, O_CREAT, 0640);
    }
}

char* expand_dollars(char* line, int lineSize) {

    // get pid
    pid_t pid = getpid();
    char pidStr[10];
    memset(pidStr, '\0', 10);
    sprintf(pidStr, "%d", pid);

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

        // if there is no $$ in string
        if (subStr == NULL)
            return tempStr;

        // allocate space for a new string
        newStr = calloc(strlen(tempStr) + strlen(pidStr) - 2 + 1, sizeof(char));
        memset(newStr, '\0', strlen(tempStr) + strlen(pidStr) - 2 + 1);

        // copy over 1st part of the string (before $$)
        int i = 0;
        while (&tempStr[i] != subStr) {
            newStr[i] = tempStr[i];
            i++;
        }

        // append the PID
        strcat(newStr, pidStr);
        // append the rest of the string (after $$)
        strcat(newStr, subStr + (2 * sizeof(char))); // increment subStr ptr over 2 places

        // free old string
        free(tempStr);
        tempStr = newStr;
    }
}