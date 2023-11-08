#include "prototypes2.h"

int main() {
    srand(time(0));
    struct Input input;
    // set all member variables of input to NULL
    initialize_input(&input);

    int status = -2; // -2 is unique so the status command knows if it is the first process to run

    char* line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;

    char* exitStr = NULL;

    int sourceFD = -5;
    int targetFD = -5;
    int result = -5;

    while (1) {
        // prompt and get input
		printf(": "); fflush(stdout);
		lineSize = getline(&line, &len, stdin); fflush(stdin);

        // blank line and comments
		if ((lineSize == 1) || (line[0] == '#')) {
			free(line);
			line = NULL;
			continue;
		}

        // expand dollars $$
		line = expand_dollars(line, lineSize);

        // tokenize the input string
        process_str(line, &input);
        // print_input(&input);


        // exit
        if ((strcmp(input.cmnd, "exit") == 0) && (input.args[1] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            // kill of child process or jobs
            printf("IN EXIT\n"); fflush(stdout);
            break;
        }

        // cd
        else if ((strcmp(input.cmnd, "cd") == 0) && (input.args[2] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            cd(input.args[1]);
            status = -1; // built-in commands exit status get ignored
        }
        
        // status
        else if ((strcmp(input.cmnd, "status") == 0) && (input.args[1] == NULL) && (input.iFile == NULL)) {
            output_status(&status, exitStr, input.oFile);
            status = -1; // built-in commands exit status get ignored
        }

        // fork and exec time baby!
        else {
            pid_t spawnpid = -5;
            int childExitMethod = -5;

            

            spawnpid = fork();

            if (spawnpid == -1) {
                perror("fork() failed, exiting\n");
                exit(1);
            }
            else if (spawnpid == 0) {
                printf("I AM THE CHILD\n"); fflush(stdout);

                // i/o redirection here
                if (input.iFile != NULL) {
                    sourceFD = open(input.iFile, O_RDONLY);
                    if (sourceFD == -1) {
                        perror("error with opening input file\n"); fflush(stderr);
                    }
                    result = dup2(sourceFD, 0);
                    if (result == -1) {
                        perror("error with source dup2()\n"); fflush(stderr);
                    }
                }

                if (input.oFile != NULL) {
                    targetFD = open(input.oFile, O_WRONLY | O_CREAT | O_TRUNC, 0760);
                    if (targetFD == -1) {
                        perror("error with opening output file\n"); fflush(stderr);
                    }
                    result = dup2(targetFD, 1);
                    if (result == -1) {
                        perror("error with target dup2()\n"); fflush(stderr);
                    }
                }

                // exec stuff
                if (execvp(input.cmnd, input.args) < 0) {
                    perror("Exec failure!\n"); fflush(stderr);
                    exit(0);
                }
            }
            else {
                if (input.bg == 0) {
                    waitpid(spawnpid, &childExitMethod, 0);
                    if (WIFEXITED(childExitMethod)) {
                        status = WEXITSTATUS(childExitMethod); // get the exit status (of the child?)
                        // exitStr = getenv("?");
                        // printf("exitStr ($?): %s\n", exitStr); fflush(stdout);
                    }
                    else {
                        // process was terminated by a signal
                    }
                    
                    printf("Parent process resumed!\n"); fflush(stdout);
                }
            }
        }



        free(line);
        line = NULL;
    }


    return 0;
}