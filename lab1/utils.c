//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define true 1
#define false 0
// given a max argument count, an array of strings and an index, the number of arguments
// until the next argument beginning with '--' is returned
int get_argument_count(int argc, char** argv, int optind){
    int count = 0;
    while(optind<argc){
        char* cmd = argv[optind];
        if(strlen(cmd) >=2 && cmd[0] == '-' && cmd[1] == '-')
            break;
        optind ++;
        count ++;
    }
    return count;
}

int are_valid_filedescriptors(char** argv, int optind, int_array* arr){
    for(int i = optind; i<optind+3; i++){
        int index = argv[i][0] - '0';
        if(index >= arr->size || arr->array[argv[i][0] - '0'] < 0)
            return false;
    }
    return true;
}

int add_filedescriptor(int_array * arr, int fd){
    if(arr->max == arr->size){
        // reallocate memory
        if(! realloc(arr->array, sizeof(int)*(arr->max + 10)))
            fprintf(stderr, "Unable to allocate more room for file descriptors: %s", strerror(errno));
        arr->max += 10;
    }
    arr->array[arr->size] = fd;
    arr->size += 1;
    return 0;
}

int redirect_input(int oldfd, int newfd){
    int nfd = dup2(oldfd, newfd);
    if(nfd<0){
        fprintf(stderr, "Unable to redirect input: %s\n", strerror(errno));
        if(fflush(stderr))
            fprintf(stderr, "Unable to flush stderr: %s", strerror(errno));
        return -1;
    }
    return 0;
}

void induce_segfault(int log){
    if(log){
        if(fprintf(stdout, "--abort\n")<0)
            fprintf(stderr, "Unable to print logging statement: %s", strerror(errno));
        if(fflush(stdout))
            fprintf(stderr, "Unable to flush stdout: %s", strerror(errno));
        fflush(stderr);
    }
    char* b = 0;
    *b = 10; //assign to a null pointer, cause segmentation fault
}

