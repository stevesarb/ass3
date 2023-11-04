#include "prototypes.h"

int main() {
	srand(time(0));
	char* line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
	
	while (1) {
		// prompt and get input
		printf(": ");
		lineSize = getline(&line, &len, stdin);

		// if (DEBUGMODE) {
		// 	printf("len = %d ; linesize = %d\n", len, lineSize);
		// 	printf("input: [%s]\n", line);
		// }

		// blank line and comments
		if ((lineSize == 1) || (line[0] == '#')) {
			free(line);
			line = NULL;
			continue;
		}	

		// expand dollars $$
			// iterate through line, serach for instances of $$
			// expand those instances, continue searching
		char* expandedStr = search_for_dollars(line, lineSize);
		// line = expandedStr;

		// exit
		if ((strncmp(line, "exit", 4) == 0) && (lineSize == 5)) {
			// kill off child processes or jobs

			break;
		}

		// cd
		// if lineSize == 4, cd is invalid (cd_) or (cdw)
		// if line[2] != ' ', then user is not trying to change directory and that command should be passed to OS
		else if ((strncmp(line, "cd", 2) == 0) && (lineSize != 4) && ((line[2] == ' ') || line[2] == '\n'))
			cd(line, lineSize);
		
		// status
		else if ((strncmp(line, "status", 6) == 0)) {
			int cmndValid = 0;
			if ((lineSize > 7) && (lineSize <= 9)) {
				// background attempt
				if ((line[6] == ' ') && (line[7] == '&')) 
					cmndValid = 1;
			}
			else if (lineSize == 7)
				cmndValid = 1;

			if (cmndValid == 1) {
				// get status
				printf("IN STATUS\n");
			}
		}

		free(line);
		line = NULL;	
	}

	return 0;
}
