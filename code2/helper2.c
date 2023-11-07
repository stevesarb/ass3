#include "prototypes2.h"

void initialize_input(struct Input* input) {
    input->cmnd = NULL;
    input->args = NULL;
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

    // first token is the command
    char* token = strtok_r(line, " \n", &savePtr); // delimited by spaces and newlines
    input->cmnd = calloc(strlen(token) + 1, sizeof(char));
    memset(input->cmnd, '\0', strlen(token) + 1);
    strcpy(input->cmnd, token);

    // if (token == NULL)
    //     return;

    // printf("token: [%s]\n", token);
    // fflush(stdout);

    int iCarrotFound = 0;
    int iFileRead = 0;
    int oCarrotFound = 0;
    int oFileRead = 0;
    int bgFound = 0;
    struct Arg* head = NULL;
    struct Arg* tail = NULL;
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

        if (strcmp(token, "&") == 0) {
            bgFound = 1;
            continue;
        }

        if ((iCarrotFound == 0) && (oCarrotFound == 0) && (bgFound == 0)) {
            // add arguments to args list
            // printf("IN ADD ARGS TO ARGS LIST\n"); fflush(stdout);
            add_arg(&head, &tail, token);
        }

        if ((iCarrotFound == 1) && (iFileRead == 0) && (bgFound == 0)) {
            // read iFile
            input->iFile = calloc(strlen(token) + 1, sizeof(char));
            memset(input->iFile, '\0', strlen(token) + 1);
            strcpy(input->iFile, token);

            iFileRead = 1;
        }

        if ((oCarrotFound == 1) && (oFileRead == 0) && (bgFound == 0)) {
            // read oFile
            input->oFile = calloc(strlen(token) + 1, sizeof(char));
            memset(input->oFile, '\0', strlen(token) + 1);
            strcpy(input->oFile, token);

            oFileRead = 1;
        }


    }

    input->args = head;

    if (iCarrotFound == 0)
        input->iFile = NULL;
    if (oCarrotFound == 0)
        input->oFile = NULL;
    if (bgFound == 1)
        input->bg = 1;
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

void add_arg(struct Arg** head, struct Arg** tail, char* token) {
    // struct Arg* head = *headPtr;
    // struct Arg* tail = *tailPtr;
    if (*head == NULL) {
        *head = malloc(sizeof(struct Arg));
        (*head)->arg = calloc(strlen(token) + 1, sizeof(char));
        memset((*head)->arg, '\0', strlen(token) + 1);
        strcpy((*head)->arg, token);
        (*head)->next = NULL;
        *tail = *head;
    }

    else {
        (*tail)->next = malloc(sizeof(struct Arg));
        *tail = (*tail)->next;
        (*tail)->arg = calloc(strlen(token) + 1, sizeof(char));
        memset((*tail)->arg, '\0', strlen(token) + 1);
        strcpy((*tail)->arg, token);
        (*tail)->next = NULL;
    }
}

void print_input(struct Input* input) {
    printf("Command: [%s]\n", input->cmnd);
    print_args(input->args);
    printf("iFile: [%s]\n", input->iFile);
    printf("oFile: [%s]\n", input->oFile);
    printf("bg: %d\n", input->bg);
    fflush(stdout);
}

void print_args(struct Arg* node) {
    if (node != NULL) {
        printf("Arg: [%s]\n", node->arg);
        print_args(node->next);
    }
}