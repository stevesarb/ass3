#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

const int DEBUGMODE = 1;


int main() {
	char* line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
	
	while (1) {
		printf(": ");
		lineSize = getline(&line, &len, stdin);

		if (DEBUGMODE)
			printf("len = %d ; linesize = %d\n", len, lineSize);
		// blank line and comments
		if ((lineSize == 1) || (line[0] == '#')) {
			free(line);
			line = NULL;
			continue;
		}

		// expand dollars $$


		// exit
		else if ((strncmp(line, "exit", 4) == 0) && (lineSize == 5))
			break;

		// cd
		// if lineSize == 4, cd is invalid (cd_) or (cdw)
		// if line[2] != ' ', then user is not trying to change directory and that command should be passed to OS
		else if ((strncmp(line, "cd", 2) == 0) && (lineSize != 4) && (line[2] == ' ')) {
			if (DEBUGMODE)
				printf("Path: [%s]\n", &line[3]);

			// cd to home directory
			if (lineSize == 3) {
				if (chdir(getenv("HOME")) != 0)
					perror("cd to home directory failed\n");
			}
			else {
				char* path = calloc(strlen(line) - 3, sizeof(char));
				strncpy(path, &line[3], lineSize - 4); // eliminates newline character
				if (chdir(path) != 0)
					perror("cd failed");
				
			}

			int fDesc;
			if (DEBUGMODE)
				fDesc = open("test.txt", O_WRONLY | O_CREAT, 0640);
		}

		free(line);
		line = NULL;	
	}

	return 0;
}
