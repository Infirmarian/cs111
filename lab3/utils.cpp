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

// Read in a block, into a dynamically allocated array of "bytes"
byte* read_block(int fd, int block_address, int block_size, int* error){
    byte* block = new byte[block_size];
    if(Pread(fd, block, sizeof(byte)*block_size, block_address*block_size)){
        *error = *error + 1;
    }
    return block;
}
