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
    char* args[512];
    char* iFile;
    char* oFile;
    int bg;
};

struct Arg {
    char* arg;
    struct Arg* next;
};


// void cd(char*, int);
void initialize_input(struct Input*);
char* expand_dollars(char*, int);
void process_str(char*, struct Input*);
void erase_input(struct Input*);
void erase_args(struct Arg*);
void add_arg(struct Arg**, struct Arg**, char*);
int determine_bg(char*, int*);
int determine_bg2(char*, int*);

void print_input(struct Input*);
void print_args(struct Arg*);


#endif