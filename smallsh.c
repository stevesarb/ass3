#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

struct Input {
    char* cmnd;
    char* args[513];
    char* iFile;
    char* oFile;
    int bg;
};

// global variables
int fgOnlyMode = 0, background = 0, childExitMethod = -5;
pid_t spawnpid = -5;

// function prototypes
char* expand_dollars(char*, int);
void process_str(char*, struct Input*);
void erase_input(struct Input*);
int determine_bg(char*, int*);
void cd(char*);
pid_t* add_pid(pid_t*, int*, int);
pid_t* check_bg(pid_t*, int*);
int* add_idx(int*, int*, int);
pid_t* remove_pids(pid_t*, int*, int*, int);
void kill_processes(pid_t*, int);
void SIGTSTP_handler(int);
void io_redirect(struct Input);
void print_input(struct Input*);


int main() {
    // variable declarations (vars associated with one another are on the same line)
    int status = 0, exitN = 1;
    char* line = NULL; size_t len = 0; ssize_t lineSize = 0;
    pid_t* pidArr = NULL; int arrSize = 0;
    struct Input input = {0};

    // parent must catch SIGTSTP (ctrl-Z)
    signal(SIGTSTP, SIGTSTP_handler);

    // parent (and background child processes) must ignore SIGINT (ctrl-C)
    signal(SIGINT, SIG_IGN);

    while (1) {
        // check on background processes
        pidArr = check_bg(pidArr, &arrSize);

        // prompt and get input
		printf(": "); fflush(stdout);
		lineSize = getline(&line, &len, stdin); fflush(stdin);

        // if program recieved a signal
        if (lineSize < 0) {
            clearerr(stdin); // remove junk from stdin (avoids uncontrolled looping)
            continue;
        }

        // blank line and comments
		if ((lineSize == 1) || (line[0] == '#')) {
			free(line); line = NULL;
			continue;
		}

        // expand dollars $$
		line = expand_dollars(line, lineSize);

        // tokenize the input string and free the string
        process_str(line, &input); free(line); line = NULL;

        // if in foreground-only mode (triggered by SIGTSTP)
        if (fgOnlyMode)
            input.bg = 0;

        // set global variable background to input.bg (for SIGTSTP signal handler)
        background = input.bg;

        // exit
        if ((strcmp(input.cmnd, "exit") == 0) && (input.args[1] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            // kill of child process or jobs
            kill_processes(pidArr, arrSize);
            exit(0);
        }

        // cd
        else if ((strcmp(input.cmnd, "cd") == 0) && (input.args[2] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) 
            cd(input.args[1]);
        
        // status
        else if ((strcmp(input.cmnd, "status") == 0) && (input.args[1] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            // if last child exited normally
            if (exitN) 
                printf("Exit value: %d\n", status);

            // if last child was terminated by a signal
            else
                printf("Terminating signal: %d\n", status);
            fflush(stdout);
        }
        
        // fork and exec time baby!
        else {
            spawnpid = fork();

            if (spawnpid == -1) {
                perror("smallsh: fork() failed, exiting\n");
                exit(1);
            }

            // child
            else if (spawnpid == 0) {

                // i/o redirection 
                io_redirect(input);

                // foreground children must not ignore SIGINT
                if (input.bg == 0) 
                    signal(SIGINT, SIG_DFL);

                // all children must ignore SIGTSTP
                signal(SIGTSTP, SIG_IGN);

                // exec
                if (execvp(input.cmnd, input.args) < 0) {
                    perror("smallsh:\nexec() failure"); fflush(stderr);
                    exit(1);
                }
            }

            // parent
            else {
                // foreground command
                if (input.bg == 0) {
                    // wait for termination
                    waitpid(spawnpid, &childExitMethod, 0);

                    // if child process exited normally
                    if (WIFEXITED(childExitMethod)) {
                        status = WEXITSTATUS(childExitMethod); // get the exit status
                        exitN = 1;
                    }
                    
                    // if child process was terminated by a signal
                    else {
                        status = WTERMSIG(childExitMethod); // get the terminating signal
                        exitN = 0;
                        printf("smallsh:\nChild process was killed by signal\n    PID: %d\n    Signal: %d\n", spawnpid, status); fflush(stdout);
                    }
                }

                // background command
                else {
                    // output PID
                    printf("Background process PID: %d\n", spawnpid); fflush(stdout);

                    // add PID to array of currently running background processes
                    pidArr = add_pid(pidArr, &arrSize, spawnpid);
                }
            }
        }
    }
}

void SIGTSTP_handler(int signum) {
    char* message = NULL;

    // wait for foreground child process to terminate
    if ((spawnpid != -5) && (background == 0)) 
        waitpid(spawnpid, &childExitMethod, 0);
    
    if (fgOnlyMode) {
        message = "\nExiting foreground-only mode\n: ";
        write(STDOUT_FILENO, message, 32);
        fgOnlyMode = 0;
    }
    else {
        message = "\nEntering foreground-only mode (& is now ignored)\n: ";
        write(STDOUT_FILENO, message, 52);
        fgOnlyMode = 1;
    }
}

char* expand_dollars(char* line, int lineSize) {
    // get pid
    pid_t pid = getpid();
    char pidStr[21]; // assuming a PID will never have more than 20 digits
    memset(pidStr, '\0', 21);
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

/*
    tokenize the input string and assign appropriate values to Input struct
*/
void process_str(char* line, struct Input* input) {
    erase_input(input);
    char* savePtr = NULL;

    int loneAmpCt = 0;
    int isBG = determine_bg(line, &loneAmpCt);
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

/* 
    determines if the command is to be run in the background and counts the 
    number of lone '&' characters there are in the input string so that the process_str 
    function can treat them as normal arguments if they are not the last '&'
*/
int determine_bg(char* line, int* loneAmpCt) {
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

/*
    resets Input struct to NULL values so the process_str function can work properly
*/
void erase_input(struct Input* input) {
    if (input->cmnd != NULL) {
        free(input->cmnd);
        input->cmnd = NULL;
    }

    int i = 0;
    while (i < 513) {
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

/*
    debugging function
*/
void print_input(struct Input* input) {
    printf("Command: [%s]\n", input->cmnd);
    int i = 0;
    while (i < 513) {
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

/*
    add an element to a dynamic array
*/
pid_t* add_pid(pid_t* pidArr, int* size, pid_t newPid) {
    pid_t* newArr = NULL;
    // if array is empty
    if (*size == 0) {
        newArr = calloc(1, sizeof(pid_t));
        newArr[0] = newPid;
    }

    // if array is not empty
    else {
        newArr = calloc(*size + 1, sizeof(pid_t));

        // copy over memory
        int i = 0;
        while (i < *size) {
            newArr[i] = pidArr[i];
            ++i;
        }
        newArr[*size] = newPid;
        free(pidArr);
    }

    *size += 1;

    return newArr;
}

/*
    before giving user command line access again, check on all 
    background processes to see if they've terminated
*/
pid_t* check_bg(pid_t* pidArr, int* size) {
    int i = 0;
    pid_t terminatedChild = -5;
    int childExitMethod = -5;
    int status;
    int* idxs = NULL;
    int idxsSize = 0;
    
    // iterate through array of PIDs
    while (i < *size) {
        // check each background child process for termination
        terminatedChild = waitpid(pidArr[i], &childExitMethod, WNOHANG);

        // if child has terminated
        if (terminatedChild != 0) {
            printf("smallsh:\nBackground process has terminated\n    PID: %d\n", terminatedChild); fflush(stdout);

            // process exited normally
            if (WIFEXITED(childExitMethod)) {
                status = WEXITSTATUS(childExitMethod); // get the exit status
                printf("    Termination method: exited normally\n    Exit value: %d\n", status); fflush(stdout);
            }

            // process was terminated by a signal
            else {
                status = WTERMSIG(childExitMethod);
                printf("    Termination method: signaled\n    Signal: %d\n", status); fflush(stdout);
            }
            
            // add index of process to index array
            idxs = add_idx(idxs, &idxsSize, i);
        }

        ++i;
    }
    
    // remove the processes that have terminated
    pidArr = remove_pids(pidArr, size, idxs, idxsSize);

    if (idxs != NULL)
        free(idxs);

    return pidArr;
}

/*
    add an element to a dynamic array
*/
int* add_idx(int* arr, int* size, int idx) {
    int* newArr = NULL;

    // if array is empty
    if (*size == 0) {
        newArr = calloc(1, sizeof(int));
        newArr[0] = idx;
    }

    // if array is not empty
    else {
        newArr = calloc(*size + 1, sizeof(int));

        // copy over memory
        int i = 0;
        while (i < *size) {
            newArr[i] = arr[i];
            ++i;
        }
        newArr[*size] = idx;
        free(arr);
    }

    *size += 1;

    return newArr;
}

/*
    remove elements from a dynamic array
    (really just add all the elements that we don't want to be removed to a new array)
*/
pid_t* remove_pids(pid_t* pidArr, int* pidSize, int* idxArr, int idxsSize) {
    int i = 0, j = 0, found = 0, newArrSize = 0;
    pid_t* newArr = NULL;

    // if there is only 1 element in pidArr to delete
    if ((*pidSize == 1) && (idxsSize == 1)) {
        free(pidArr);
        *pidSize -= 1;
        return NULL;
    }

    // iterate through pidArr
    while (i < *pidSize) {
        j = 0;
        found = 0;
        // iterate through idxArr
        while (j < idxsSize) {
            // if the current element is the pid of a terminated child process
            if (i = idxArr[j]) {
                found = 1;
                break;
            }
            ++j;
        }

        // if the current element is not one to be deleted, add it to a new array
        if (found == 0) 
            newArr = add_pid(newArr, &newArrSize, pidArr[i]);
        
        ++i;
    }

    // all the memory we want to keep should be copied over to newArr
    if (pidArr != NULL)
        free(pidArr);

    *pidSize = newArrSize;

    return newArr;
}

/*
    before exiting smallsh, kill and reap all child processes
*/
void kill_processes(pid_t* arr, int size) {
    int i = 0, terminatedChild = -5, childExitMethod = -5, killResult = -5;

    // iterate through array of PIDs
    while (i < size) {
        // determine if process has already terminated or not
        terminatedChild = waitpid(arr[i], &childExitMethod, WNOHANG);

        // if it has not been terminated
        if (terminatedChild == 0) {
            // kill the process
            killResult = kill(arr[i], SIGKILL);

            // reap it
            terminatedChild = waitpid(arr[i], &childExitMethod, 0);
        }
        ++i;
    }
}

/*
    child performs i/o redirection before executing itself
*/
void io_redirect(struct Input input) {
    int sourceFD = -5, targetFD = -5, result = -5, devNull = -5;

    // if user specified an input file
    if (input.iFile != NULL) {
        sourceFD = open(input.iFile, O_RDONLY);
        if (sourceFD < 0) {
            perror("smallsh:\nerror with opening input file"); fflush(stderr);
            exit(1);
        }
        result = dup2(sourceFD, 0);
        if (result == -1) {
            perror("smallsh:\nerror with source dup2()\n"); fflush(stderr);
        }
    }

    // if user did not specify an input file and this command is to be run in the background
    else if ((input.iFile == NULL) && (input.bg == 1)) {
        devNull = open("/dev/null", O_RDWR);
        if (devNull < 0) {
            perror("smallsh:\nerror with opening /dev/null\n"); fflush(stderr);
            exit(1);
        }
        result = dup2(devNull, 0);
        if (result == -1) {
            perror("smallsh:\nerror with /dev/null dup2()\n"); fflush(stderr);
            exit(1);
        }
    }

    // if user specified an output file
    if (input.oFile != NULL) {
        targetFD = open(input.oFile, O_WRONLY | O_CREAT | O_TRUNC, 0760);
        if (targetFD < 0) {
            perror("smallsh:\nerror with opening output file"); fflush(stderr);
            exit(1);
        }
        result = dup2(targetFD, 1);
        if (result == -1) {
            perror("smallsh:\nerror with target dup2()\n"); fflush(stderr);
            exit(1);
        }
    }

    // if user did not specify an output file and this command is to be run in the background
    else if ((input.oFile == NULL) && (input.bg == 1)) {
        // if file is not already open
        if (devNull == -5) {
            devNull = open("/dev/null", O_RDWR);
            if (devNull < 0) {
                perror("smallsh:\nerror with opening /dev/null\n"); fflush(stderr);
                exit(1);
            }
            result = dup2(devNull, 1);
            if (result == -1) {
                perror("smallsh:\nerror with /dev/null dup2()\n"); fflush(stderr);
                exit(1);
            }
        }

        // if file is already open
        else {
            result = dup2(devNull, 1);
            if (result == -1) {
                perror("smallsh:\nerror with /dev/null dup2()\n"); fflush(stderr);
                exit(1);
            }
        }
    }
}