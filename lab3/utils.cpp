//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <string>
#include <time.h>

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
byte* read_block(int fd, int block_address, int block_size, int& error){
    byte* block = new byte[block_size];
    if(Pread(fd, block, sizeof(byte)*block_size, block_address*block_size)){
        error += 1;
    }
    return block;
}
// Read in multiple blocks into a dynamically allocated array
byte* read_blocks(int fd, int block_address, int block_size, int block_count, int& error){
    byte* block = new byte[block_size*block_count];
    if(Pread(fd, block, sizeof(byte)*block_size*block_count, block_address*block_size))
        error++;
    return block;
}

bool inode_is_free(std::vector<byte> inodes, int number){
    number --;
    int vecSpot = number / 8;
    int bitspot = number % 8;
    return !(inodes[vecSpot] & 1 << bitspot);
}
bool inode_is_free(byte* b, int number){
    number --;
    int byteSpot = number /8;
    int bitspot = number % 8;
    return !(b[byteSpot] & 1 << bitspot);
}

bool block_is_free(std::vector<byte> blocks, int number){
    return inode_is_free(blocks, number);
}
bool block_is_free(byte* blocks, int number){
    return inode_is_free(blocks, number);
}

std::string convert_to_time(long int total_seconds){
    struct tm* time = gmtime(&total_seconds);
    char buf[30];
    sprintf(buf, "%02d/%02d/%02d %02d:%02d:%02d",
        time->tm_mon + 1, time->tm_mday, time->tm_year%100, time->tm_hour,
        time->tm_min, time->tm_sec );
    std::string res = "";
    res.append(buf);
    return res;
}

