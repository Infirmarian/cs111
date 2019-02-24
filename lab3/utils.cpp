//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include "utils.h"


// Utility to cleanly flush and check an output
void Fflush(FILE* f){
    if(fflush(f)){
        fprintf(stderr, "Unable to flush to file: %s\n", strerror(errno));
    }
}

int Pread(int fd, void* ptr, size_t size, off_t offset){
    if(pread(fd, ptr, size, offset) == -1){
        fprintf(stderr, "Unable to read input from given file: %s", strerror(errno));
        Fflush(stderr);
        return 1;
    }
    return 0;
}