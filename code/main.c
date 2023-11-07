#include "prototypes.h"

int main() {
	srand(time(0));
	char* line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
	int status = -2; // -2 is unique so the status command knows if it is the first process to run
	
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

		// exit
		// if lineSize == 7, then user typed "status &" in an attempt to run exit in the background
		// if lineSize == 7 and the last 2 characters are not ' ' and '&', then command is invalid
		if ((strncmp(line, "exit", 4) == 0) && ((lineSize == 5) || ((lineSize == 7) && (line[4] == ' ') && (line[5] == '&')))) {
			// kill off child processes or jobs
			printf("IN EXIT\n"); fflush(stdout);
			
			break;
		}

		// cd
		// if lineSize == 4, cd is invalid (cd_) or (cdw)
		// if line[2] != ' ' or '\n', then user is not trying to change directory and that command should be passed to OS
		else if ((strncmp(line, "cd", 2) == 0) && (lineSize != 4) && ((line[2] == ' ') || line[2] == '\n')) {
			cd(line, lineSize);
			status = -1;
		}
		
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
				fflush(stdout);

				// if status is run before any other foreground command
				if (status == -2) {
					printf("exit value %d\n", 0);
				}

				// if last foreground command run was not a built-in command
				else if (status != -1) {
					printf("exit value %d\n", getenv("?"));
				}
				
				// built-in commands get status of -1
				status = -1;
			}
		}

		else if (strncmp(line, "myCommand", 9)) {
			status = 1;
		}

		free(line);
		line = NULL;	
	}

	return 0;
}
