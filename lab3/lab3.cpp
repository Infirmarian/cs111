//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "utils.h"


std::string getSuperblockData(int fd, int* error){
    // Located 1024 bytes from the beginning of the filesystem
    std::string result = "SUPERBLOCK,";
    int superblockAddress = 1024;
    int inode_count, block_count, block_size, blocks_per_group, inodes_per_group, first_inode;
    short inode_size;
    int e = 0;
    e += Pread(fd, &block_count, sizeof(int), superblockAddress);
    e += Pread(fd, &inode_count, sizeof(int), superblockAddress+4);

    // block size needs to be 1024 shifted by the value on disk
    e += Pread(fd, &block_size, sizeof(int), superblockAddress+24);
    block_size = 1024 << block_size;

    e += Pread(fd, &inode_size, sizeof(short), superblockAddress + 88);
    e += Pread(fd, &blocks_per_group, sizeof(int), superblockAddress + 32);
    e += Pread(fd, &inodes_per_group, sizeof(int), superblockAddress + 40);
    e += Pread(fd, &first_inode, sizeof(int), superblockAddress + 84);

    result  +=  std::to_string(block_count)+","+std::to_string(inode_count)+","+std::to_string(block_size)+
                ","+std::to_string(inode_size) + "," + std::to_string(blocks_per_group) + "," +
                std::to_string(inodes_per_group) + "," + std::to_string(first_inode);
    *error = e;
    return result;
}

int main(int argc, char** argv){
    int exit_status = 0;
    if(argc != 2){
        fprintf(stderr, "Usage: ./lab3 <DISK IMAGE>\n");
        Fflush(stdout);
        exit(2);
    }
    int img_fd = open(argv[1], O_RDONLY);
    // check that the specified disk image could be opened
    if(img_fd < 0){
        fprintf(stderr, "Unable to open provided disk image: %s\n", strerror(errno));
        Fflush(stderr);
        exit(2);
    }
    int e = 0;
    // Produce the series of output required by the program
    fprintf(stdout, "%s\n", getSuperblockData(img_fd, &e).c_str());
    if(e > 0){
        exit_status = 1;
    }

    if(close(img_fd)){
        fprintf(stderr, "Unable to close disk image file: %s\n", strerror(errno));
        Fflush(stderr);
        exit_status = 1;
    }
    return exit_status;
}