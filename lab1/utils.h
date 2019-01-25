//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

typedef struct{
    int max;
    int size;
    int* array;
} int_array;

typedef struct{
    pid_t pid;
    char** arg;
    int argc;
} com_t;

typedef struct{
    int max;
    int size;
    com_t * array;
} proc_array;

int get_argument_count(int argc, char** argv, int optind);
int are_valid_filedescriptors(char** argv, int optind, int_array* arr);
int add_int(int_array * arr, int fd);
int add_proc(proc_array* arr, com_t com);
int redirect_input(int oldfd, int newfd);
void induce_segfault(int log);
#endif