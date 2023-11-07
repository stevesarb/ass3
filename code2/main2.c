#include "prototypes2.h"

int main() {
    struct Input input;

    // set input struct values all to NULL

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
    }


    return 0;
}