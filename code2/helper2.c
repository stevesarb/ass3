#include "prototypes2.h"

void initialize_input(struct Input* input) {
    input->cmnd = NULL;
    int i = 0;
    while (i < 512) {
        input->args[i] = NULL;
        ++i;
    }
    input->iFile = NULL;
    input->oFile = NULL;
    input->bg = 0;
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

void process_str(char* line, struct Input* input) {
    erase_input(input);
    char* savePtr = NULL;

    int loneAmpCt = 0;
    int isBG = determine_bg2(line, &loneAmpCt);
    input->bg = isBG;

    // first token is the command
    char* token = strtok_r(line, " \n", &savePtr); // delimited by spaces and newlines
    input->cmnd = calloc(strlen(token) + 1, sizeof(char));
    memset(input->cmnd, '\0', strlen(token) + 1);
    strcpy(input->cmnd, token);
    
    // set first argument to be the same as the command
    input->args[0] = calloc(strlen(token) + 1, sizeof(char));
    memset(input->args[0], '\0', strlen(token) + 1);
    strcpy(input->args[0], token);


    int iCarrotFound = 0;
    int iFileRead = 0;
    int oCarrotFound = 0;
    int oFileRead = 0;
    int tokenAmpCt = 0;
    int i = 1;
    while (token != NULL) {
        token = strtok_r(NULL, " \n", &savePtr);

        if (token == NULL)
            break;

        if (strcmp(token, "<") == 0) {
            iCarrotFound = 1;
            continue;
        }

        if (strcmp(token, ">") == 0) {
            oCarrotFound = 1;
            continue;
        }

        // add & to args list if they are not at the end of the string
        if ((strcmp(token, "&") == 0) && (isBG == 1)) {
            ++tokenAmpCt;
            if (tokenAmpCt == loneAmpCt) 
                continue;
        }

        if ((iCarrotFound == 0) && (oCarrotFound == 0)) {
            // add arguments to args list
            input->args[i] = calloc(strlen(token) + 1, sizeof(char));
            memset(input->args[i], '\0', strlen(token) + 1);
            strcpy(input->args[i], token);
            ++i;
        }

        if ((iCarrotFound == 1) && (iFileRead == 0)) {
            // read iFile
            input->iFile = calloc(strlen(token) + 1, sizeof(char));
            memset(input->iFile, '\0', strlen(token) + 1);
            strcpy(input->iFile, token);

            iFileRead = 1;
        }

        if ((oCarrotFound == 1) && (oFileRead == 0)) {
            // read oFile
            input->oFile = calloc(strlen(token) + 1, sizeof(char));
            memset(input->oFile, '\0', strlen(token) + 1);
            strcpy(input->oFile, token);

            oFileRead = 1;
        }
    }

    if (iCarrotFound == 0)
        input->iFile = NULL;
    if (oCarrotFound == 0)
        input->oFile = NULL;
}

int determine_bg2(char* line, int* loneAmpCt) {
    char* savePtr = NULL;
    char* lineCopy = calloc(strlen(line) + 1, sizeof(char));
    memset(lineCopy, '\0', strlen(line) + 1);
    strcpy(lineCopy, line);
    char* token = strtok(lineCopy, " \n");

    while (token != NULL) {
        if (strcmp(token, "&") == 0) 
            *loneAmpCt += 1;
        
        token = strtok(NULL, " \n");
    }

    // valid background command
    if ((line[strlen(line) - 2] == '&') && (line[strlen(line) - 3] == ' '))
        return 1;
    else 
        return 0;
}

void erase_input(struct Input* input) {
    if (input->cmnd != NULL) {
        free(input->cmnd);
        input->cmnd = NULL;
    }

    int i = 0;
    while (i < 512) {
        if (input->args[i] != NULL) {
            free(input->args[i]);
            input->args[i] = NULL;
        }
        ++i;
    }

    if (input->iFile != NULL) {
        free(input->iFile);
        input->iFile = NULL;
    }

    if (input->oFile != NULL) {
        free(input->oFile);
        input->oFile = NULL;
    }

    if (input->bg == 1)
        input->bg = 0;
}

void print_input(struct Input* input) {
    printf("Command: [%s]\n", input->cmnd);
    // print_args(input->args);
    int i = 0;
    while (i < 512) {
        if (input->args[i] != NULL) {
            printf("Arg %d: [%s]\n", i + 1, input->args[i]);
            fflush(stdout);
        }
        ++i;
    }
    printf("iFile: [%s]\n", input->iFile);
    printf("oFile: [%s]\n", input->oFile);
    printf("bg: %d\n", input->bg);
    fflush(stdout);
}

void cd(char* path) {
    // cd to home directory
    if (path == NULL) 
        chdir(getenv("HOME"));
    else {
        // if cd with absolute path from home directory
        if (path[0] == '~') {
            char* homePath = getenv("HOME");
            // allocate space for new string
            char* fullPath = calloc(strlen(homePath) + strlen(path), sizeof(char));
            memset(fullPath, '\0', strlen(homePath) + strlen(path));
            // cat homePath to fullPath
            strcat(fullPath, homePath);
            // cat remainder of path (after ~) to fullPath
            strcat(fullPath, &path[1]);
            // cd
            if (chdir(fullPath) != 0) 
                perror("");   
        }

        // cd to specified path (relative or absolute)
        else
            if (chdir(path) != 0) 
                perror("");       
    }
}

void output_status(int* status, char* exitStr, char* oFile) {
    // char* exitVal = NULL;
    int exitVal;
    FILE* file = NULL;
    // if status is run before any other foreground command
    if (*status == -2) {
        exitVal = 0;
        // exitVal = calloc(2, sizeof(char));
        // memset(exitVal, '\0', strlen(exitVal));
        // exitVal[0] = '0';
    }

    // if last foreground command run was not a built-in command
    else if (*status != -1) {
        // exitVal = getenv("?");
        // exitVal = exitStr;
        exitVal = *status;
    }

    if (*status != -1) {
        // if no output redirection file was specified
        if (oFile == NULL) {
            printf("Exit value: %d\n", exitVal); fflush(stdout);
        }
            
        else {
            // write status to file
            file = fopen(oFile, "w");
            if (file == NULL) {
                perror("Failed to open specified output file\n"); fflush(stderr);
                *status = 1;
                return;
            }
            else {
                fprintf(file, "%d", exitVal);
                fclose(file);
            }
        }
    }
}