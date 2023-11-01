#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int DEBUGMODE = 0;


int main() {
	char* line = NULL;
    size_t len = 0;
    ssize_t lineSize = 0;
	
	while (1) {
		printf(": ");
		lineSize = getline(&line, &len, stdin);
		if (DEBUGMODE)
			printf("\nlen = %d ; linesize = %d\n", len, lineSize);

		if ((strncmp(line, "exit", 4) == 0) && (lineSize == 5))
			break;
		free(line);
		line = NULL;
	}

	return 0;
}
