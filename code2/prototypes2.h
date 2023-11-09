#ifndef PROTOTYPES2_H
#define PROTOTYPES2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

// global variables
struct sigaction SIGTSTP_action1 = {0}, SIGTSTP_action2 = {0};

struct Input {
    char* cmnd;
    char* args[512];
    char* iFile;
    char* oFile;
    int bg;
};


void initialize_input(struct Input*);
char* expand_dollars(char*, int);
void process_str(char*, struct Input*);
void erase_input(struct Input*);
int determine_bg2(char*, int*);
void output_status(int*, char*, char*);
void cd(char*);
pid_t* add_pid(pid_t*, int*, int);
pid_t* check_bg(pid_t*, int*);
int* add_idx(int*, int*, int);
pid_t* remove_pids(pid_t*, int*, int*, int);
void kill_processes(pid_t*, int);
void catchSIGTSTP1(int);
void catchSIGTSTP2(int);

void print_input(struct Input*);


#endif