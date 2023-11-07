#include "prototypes2.h"

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
    erase_input(&input);
    char* savePtr = NULL;

    // first token is the command
    char* token = strtok_r(line, " \n", &savePtr); // delimited by spaces and newlines
    input->cmnd = calloc(strlen(token) + 1, sizeof(char));
    memset(input->cmnd, '\0', strlen(token) + 1);
    strcpy(input->cmnd, token);

    if (token == NULL)
        return;

    printf("token: [%s]\n", token);
    fflush(stdout);

    int iCarrotFound = 0;
    int iFileRead = 0;
    int oCarrotFound = 0;
    int oFileRead = 0;
    int bgFound = 0;
    while (token != NULL) {
        token = strtok_r(NULL, " \n", &savePtr);

        if (token == NULL)
            break;

        if (strcmp(token, "<") == 0) {
            iCarrotFound = 1;
        }

        // THESE NEED TO BE ELSE IF's


        if (strcmp(token, ">") == 0) {
            oCarrotFound = 1;
        }

        if (strcmp(token, "&") == 0) {
            bgFound = 1;
        }

        if ((iCarrotFound == 0) && (oCarrotFound == 0) && (bgFound == 0)) {
            // add arguments to args list
        }

        if ((iCarrotFound == 1) && (iFileRead == 0) && (bgFound == 0)) {
            // read iFile

            iFileRead = 1;
        }

        if ((oCarrotFound == 1) && (oFileRead == 0) && (bgFound == 0)) {
            // read oFile

            oFileRead = 1;
        }


    }

    if (iCarrotFound == 0)
        input.iFile = NULL;
    if (oCarrotFound == 0)
        input.oFile = NULL;
}

void erase_input(struct Input* input) {
    if (input->cmnd != NULL) {
        free(input->cmnd);
        input->cmnd = NULL;
    }

    if (input->args != NULL) {
        erase_args(input->args);
        input->args = NULL;
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

void erase_args(struct Arg* arg) {
    if (arg->next != NULL) {
        erase_args(arg->next);
    }
    free(arg);
}