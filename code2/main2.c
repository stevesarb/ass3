#include "prototypes2.h"

int main() {
    srand(time(0));
    struct Input input;
    // set all member variables of input to NULL
    initialize_input(&input);

    int status = -2;

    char* line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;

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
        if ((strcmp(input.cmnd, "exit") == 0) && (input.args[0] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            // kill of child process or jobs
            printf("IN EXIT\n"); fflush(stdout);
            break;
        }

        // cd
        if ((strcmp(input.cmnd, "cd") == 0) && (input.args[1] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            cd(input.args[0]);
            status = -1;
        }
        
        // status
        if ((strcmp(input.cmnd, "status") == 0) && (input.args[0] == NULL) && (input.iFile == NULL)) {
            // char exitVal[4] = {'\0', '\0', '\0', '\0'};
            char* exitVal = NULL;

            // if status is run before any other foreground command
            if (status == -2) {
                exitVal = calloc(2, sizeof(char));
                memset(exitVal, '\0', strlen(exitVal));
                exitVal[0] = '0';
            }

            // if last foreground command run was not a built-in command
            else if (status != -1)
                exitVal = getenv("?");

            if (status != -1) {
                // if no output redirection file was specified
                if (input.oFile == NULL) {
                    printf("Exit value: %s\n", exitVal); fflush(stdout);
                }
                    
                else {
                    // write status to file
                    int fileDesc = open(input.oFile, O_WRONLY | O_CREAT | O_TRUNC, 0640);
                    int nWritten = write(fileDesc, exitVal, strlen(exitVal) * sizeof(char));
                    close(fileDesc);
                }
            }
            status = -1;
            free(exitVal);
        }



        free(line);
        line = NULL;
    }


    return 0;
}