#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

struct Input {
    char* cmnd;
    struct Arg* args;
    char* iFile;
    char* oFile;
    int bg;
};

struct Arg {
    char* arg;
    struct Arg* next;
};


// void cd(char*, int);

char* expand_dollars(char*, int);
void process_str(char*, struct Input*);
void erase_input(struct Input*);
void erase_args(struct Arg*);


#endif