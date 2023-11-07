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
        print_input(&input);

        // exit
        if ((strcmp(input.cmnd, "exit") == 0) && (input.args[0] == NULL) && (input.iFile == NULL) && (input.oFile == NULL)) {
            // kill of child process or jobs
            printf("IN EXIT\n"); fflush(stdout);

            break;
        }

        // cd
        if ((strcmp(input.cmnd, "cd") == 0) && (input.args[1] == NULL) && (input.iFile == NULL) && (input.oFile == NULL))
            cd(input.args[0]);
        
        if ((strcmp(input.cmnd, "status") == 0))

        free(line);
        line = NULL;
    }


    return 0;
}