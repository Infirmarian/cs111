//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "utils.h"

#define SUPERBLOCK_LOCATION 1024

static int BLOCK_SIZE = 1024;

std::string getSuperblockData(int fd, int* error){
    // Located 1024 bytes from the beginning of the filesystem
    std::string result = "SUPERBLOCK,";
    int inode_count, block_count, block_size, blocks_per_group, inodes_per_group, first_inode;
    short inode_size;
    int e = 0;
    e += Pread(fd, &block_count, sizeof(int), SUPERBLOCK_LOCATION+4);
    e += Pread(fd, &inode_count, sizeof(int), SUPERBLOCK_LOCATION);

    // block size needs to be 1024 shifted by the value on disk
    e += Pread(fd, &block_size, sizeof(int), SUPERBLOCK_LOCATION+24);
    BLOCK_SIZE = block_size = 1024 << block_size;

    e += Pread(fd, &inode_size, sizeof(short), SUPERBLOCK_LOCATION + 88);
    e += Pread(fd, &blocks_per_group, sizeof(int), SUPERBLOCK_LOCATION + 32);
    e += Pread(fd, &inodes_per_group, sizeof(int), SUPERBLOCK_LOCATION + 40);
    e += Pread(fd, &first_inode, sizeof(int), SUPERBLOCK_LOCATION + 84);

    result  +=  std::to_string(block_count)+","+std::to_string(inode_count)+","+std::to_string(block_size)+
                ","+std::to_string(inode_size) + "," + std::to_string(blocks_per_group) + "," +
                std::to_string(inodes_per_group) + "," + std::to_string(first_inode);
    *error = e;
    return result;
}

std::string getGroupData(int fd, int* error, int groupNumber){
    std::string result = "GROUP,"; // Guaranteed to only have 1 Group
    int groupTableOffset = SUPERBLOCK_LOCATION + BLOCK_SIZE + 32 * groupNumber;
    int group_number;
    int e = 0;
    e += Pread(fd, &group_number, sizeof(int), groupTableOffset);
    *error += e;
    result += std::to_string(groupNumber);
    return result;
}

int main(int argc, char** argv){
    int exit_status = 0;
    if(argc != 2){
        fprintf(stderr, "Usage: ./lab3 <DISK IMAGE>\n");
        Fflush(stdout);
        exit(1);
    }
    int img_fd = open(argv[1], O_RDONLY);
    // check that the specified disk image could be opened
    if(img_fd < 0){
        fprintf(stderr, "Unable to open provided disk image: %s\n", strerror(errno));
        Fflush(stderr);
        exit(1);
    }
    int e = 0;
    // Produce the series of output required by the program
    fprintf(stdout, "%s\n", getSuperblockData(img_fd, &e).c_str()); //SUPERBLOCK
    fprintf(stdout, "%s\n", getGroupData(img_fd, &e, 0).c_str()); // GROUP DATA

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