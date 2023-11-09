#include "helper2.c"

int main() {

    // signal stuff
    SIGTSTP_action1.sa_handler = catchSIGTSTP1;
	sigfillset(&SIGTSTP_action1.sa_mask);
	SIGTSTP_action1.sa_flags = 0;
    sigaction(SIGTSTP, &SIGTSTP_action1, NULL);

    SIGTSTP_action2.sa_handler = catchSIGTSTP2;
	sigfillset(&SIGTSTP_action2.sa_mask);
	SIGTSTP_action2.sa_flags = 0;
    // sigaction(SIGTSTP, &SIGTSTP_action2, NULL);

    signal(SIGINT, SIG_IGN);


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

    pid_t* pidArr = NULL;
    int arrSize = 0;

    while (1) {
        // check on background processes
        // sleep(1);
        pidArr = check_bg(pidArr, &arrSize);

        // prompt and get input
		printf(": "); fflush(stdout);
		lineSize = getline(&line, &len, stdin); fflush(stdin);

        // line size will be < 0 if program recieved a signal
        if (lineSize < 0) {
            clearerr(stdin); // remove junk from stdin (avoids uncontrolled looping)
            continue;
        }

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
            kill_processes(pidArr, arrSize);
            break;
        }

        // cd
        else if ((strcmp(input.cmnd, "cd") == 0) && (input.args[2] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            cd(input.args[1]);
        }
        
        // status
        else if ((strcmp(input.cmnd, "status") == 0) && (input.args[1] == NULL) && (input.iFile == NULL)) {
            output_status(&status, exitStr, input.oFile);
        }

        // fork and exec time baby!
        else {
            pid_t spawnpid = -5;
            int childExitMethod = -5;
            int devNull = -5;
            

            spawnpid = fork();

            if (spawnpid == -1) {
                perror("smallsh: fork() failed, exiting\n");
                exit(1);
            }

            // child
            else if (spawnpid == 0) {
                // printf("I AM THE CHILD\n"); fflush(stdout);
                devNull = -5;

                // i/o redirection here

                // if user specified an input file
                if (input.iFile != NULL) {
                    sourceFD = open(input.iFile, O_RDONLY);
                    if (sourceFD < 0) {
                        perror("smallsh: error with opening input file"); fflush(stderr);
                        // status = 1; // this would set the status of the child, not the parent
                        exit(1);
                    }
                    result = dup2(sourceFD, 0);
                    if (result == -1) {
                        perror("error with source dup2()\n"); fflush(stderr);
                    }
                }

                // if user did not specify an input file and this command is to be run in the background
                else if ((input.iFile == NULL) && (input.bg == 1)) {
                    devNull = open("/dev/null", O_RDWR);
                    if (devNull < 0) {
                        perror("smallsh: error with opening /dev/null\n"); fflush(stderr);
                        // status = 1; // this would set the status of the child, not the parent
                        exit(1);
                    }
                    result = dup2(devNull, 0);
                    if (result == -1) {
                        perror("smallsh: error with /dev/null dup2()\n"); fflush(stderr);
                        // status = 1; // this would set the status of the child, not the parent
                        exit(1);
                    }
                }

                // if user specified an output file
                if (input.oFile != NULL) {
                    targetFD = open(input.oFile, O_WRONLY | O_CREAT | O_TRUNC, 0760);
                    if (targetFD < 0) {
                        perror("smallsh: error with opening output file"); fflush(stderr);
                        // status = 1; // this would set the status of the child, not the parent
                        exit(1);
                    }
                    result = dup2(targetFD, 1);
                    if (result == -1) {
                        perror("smallsh: error with target dup2()\n"); fflush(stderr);
                        // status = 1; // this would set the status of the child, not the parent
                        exit(1);
                    }
                }

                // if user did not specify an output file and this command is to be run in the background
                else if ((input.oFile == NULL) && (input.bg == 1)) {
                    // if file is not already open
                    if (devNull == -5) {
                        devNull = open("/dev/null", O_RDWR);
                        if (devNull < 0) {
                            perror("smallsh: error with opening /dev/null\n"); fflush(stderr);
                            // status = 1; // this would set the status of the child, not the parent
                            exit(1);
                        }
                        result = dup2(devNull, 1);
                        if (result == -1) {
                            perror("smallsh: error with /dev/null dup2()\n"); fflush(stderr);
                            // status = 1; // this would set the status of the child, not the parent
                            exit(1);
                        }
                    }

                    // if file is already open
                    else {
                        result = dup2(devNull, 1);
                        if (result == -1) {
                            perror("smallsh: error with /dev/null dup2()\n"); fflush(stderr);
                            // status = 1; // this would set the status of the child, not the parent
                            exit(1);
                        }
                    }
                }

                // if child is to be run in the foreground, don't ignore SIGINT
                if (input.bg == 0) {
                    signal(SIGINT, SIG_DFL);
                }

                // all children must ignore SIGTSTP
                signal(SIGTSTP, SIG_IGN);

                // exec stuff
                if (execvp(input.cmnd, input.args) < 0) {
                    perror("smallsh: exec() failure"); fflush(stderr);
                    exit(1);
                }
            }
            // parent
            else {
                // foreground command
                if (input.bg == 0) {
                    waitpid(spawnpid, &childExitMethod, 0);
                    if (WIFEXITED(childExitMethod)) {
                        status = WEXITSTATUS(childExitMethod); // get the exit status (of the child?)
                    }
                    else {
                        // process was terminated by a signal
                        status = WTERMSIG(childExitMethod);
                        printf("smallsh:\nChild process was killed by signal\n    PID: %d\n    Signal: %d\n", spawnpid, status);
                    }
                }

                // background command
                else {
                    printf("Background process PID: %d\n", spawnpid); fflush(stdout);
                    pidArr = add_pid(pidArr, &arrSize, spawnpid);
                }
            }
        }



        free(line);
        line = NULL;
    }


    return 0;
}