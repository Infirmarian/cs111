//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <vector>

#include "utils.h"

#define SUPERBLOCK_LOCATION 1

static int BLOCK_SIZE = 1024;
static byte* superblockdata = 0;
struct groupData{
    short free_block_count;
    short free_inode_count;
    int block_bitmap_address;
    int inode_bitmap_address;
    int first_inode_block;
    int block_count;
    int inode_count;
};
std::vector<struct groupData*> groups;

std::string getSuperblockData(int fd, int* error){
    // Located 1024 bytes from the beginning of the filesystem
    std::string result = "SUPERBLOCK,";
    byte* b = read_block(fd, SUPERBLOCK_LOCATION, BLOCK_SIZE, error);
    superblockdata = b;
    int inode_count, block_count, block_size, blocks_per_group, inodes_per_group, first_inode;
    short inode_size;
    inode_count = convert_bytes_to_type<int>(b,0);
    block_count = convert_bytes_to_type<int>(b,4);
    block_size = convert_bytes_to_type<int>(b, 24);
    // Block size needs to be 1024 shifted by block size
    BLOCK_SIZE = block_size = 1024 << block_size;

    inode_size = convert_bytes_to_type<short>(b,88);
    blocks_per_group = convert_bytes_to_type<int>(b, 32);
    inodes_per_group = convert_bytes_to_type<int>(b, 40);
    first_inode = convert_bytes_to_type<int>(b, 84);

    result  +=  std::to_string(block_count)+","+std::to_string(inode_count)+","+std::to_string(block_size)+
                ","+std::to_string(inode_size) + "," + std::to_string(blocks_per_group) + "," +
                std::to_string(inodes_per_group) + "," + std::to_string(first_inode);
    // don't delete the superblock data
    return result;
}

std::string getGroupData(int fd, int groupNumber, int* error){
    std::string result = "GROUP,"; // Guaranteed to only have 1 Group
    byte* b = read_block(fd, SUPERBLOCK_LOCATION + 1, BLOCK_SIZE, error);
    int groupOffset = groupNumber * 32;
    short free_block_count, free_inode_count;
    int block_bitmap_address, inode_bitmap_address, first_inode_block;

    free_block_count = convert_bytes_to_type<short>(b, groupOffset + 12);
    free_inode_count = convert_bytes_to_type<short>(b, groupOffset + 14);
    block_bitmap_address = convert_bytes_to_type<int>(b, groupOffset + 0);
    inode_bitmap_address = convert_bytes_to_type<int>(b, groupOffset + 4);
    first_inode_block = convert_bytes_to_type<int>(b, groupOffset + 8);

    // Save data to a struct, for future reference
    struct groupData* d = new struct groupData;
    d->free_block_count = free_block_count;
    d->free_inode_count = free_inode_count;
    d->block_bitmap_address = block_bitmap_address;
    d->inode_bitmap_address = inode_bitmap_address;
    d->first_inode_block = first_inode_block;

    // Gather information from the superblock, if it hasn't already been done
    if(!superblockdata){
        superblockdata = read_block(fd, SUPERBLOCK_LOCATION, BLOCK_SIZE, error);
    }
    int block_count = convert_bytes_to_type<int>(superblockdata, 4);
    int blocks_per_group = convert_bytes_to_type<int>(superblockdata, 32);
    int inodes_per_group = convert_bytes_to_type<int>(superblockdata, 40);

    block_count = block_count % blocks_per_group; // only for the last group
    int inode_count = inodes_per_group;

    d->block_count = block_count;
    d->inode_count = inode_count;


    result += std::to_string(groupNumber)+","+std::to_string(block_count)+","+std::to_string(inode_count)+
            ","+std::to_string(free_block_count)+","+ std::to_string(free_inode_count) + 
            ","+std::to_string(block_bitmap_address) + ","+std::to_string(inode_bitmap_address) + 
            ","+ std::to_string(first_inode_block);

    std::vector<struct groupData*>::iterator it = groups.begin();
    for(unsigned int i = 0; i<groupNumber - groups.size(); i++){
            groups.push_back(0);
    }
    for(int i = 0; i<groupNumber; i++){
        it++;
    }
    groups.insert(it, d);
    delete[] b;
    return result;
}

std::string getFreeBlockData(int fd, int groupNumber, int* error);
std::string getFreeInodeData(int fd, int groupNumber, int* error);

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
    fprintf(stdout, "%s\n", getGroupData(img_fd, 0, &e).c_str()); // GROUP DATA
    fprintf(stdout, "%s", getFreeBlockData(img_fd, 0, &e).c_str()); // Block DATA
    fprintf(stdout, "%s", getFreeInodeData(img_fd, 0, &e).c_str()); // Inode DATA

    if(e > 0){
        exit_status = 1;
    }

    if(close(img_fd)){
        fprintf(stderr, "Unable to close disk image file: %s\n", strerror(errno));
        Fflush(stderr);
        exit_status = 1;
    }
    if(superblockdata){
        delete[] superblockdata;
    }
    for(unsigned int i = 0; i<groups.size(); i++){
        delete groups[i];
    }
    return exit_status;
}

// Return a string that represents each free block in the given group
std::string getFreeBlockData(int fd, int groupNumber, int* error){
    std::string result = "";
    int bitmap_pos = (groups[groupNumber])->block_bitmap_address;
    int block_count = groups[groupNumber]->block_count;
    byte* b = read_block(fd, bitmap_pos, BLOCK_SIZE, error);
    long long count = 1;
    int shift = 0;
    for(int i = 0; count <= block_count; i++){
        for(int j = 0; j<8 && (i<<3)+j <= block_count; j++){
            if(!(b[i] & (1 << shift))){
                result += "BFREE,"+std::to_string(count)+"\n";
            }
            shift ++;
            count++;
        }
        shift = 0;
    }

    delete[] b;
    return result;
}

// Return a string that represents each free block in the given group
std::string getFreeInodeData(int fd, int groupNumber, int* error){
    std::string result = "";
    int bitmap_pos = (groups[groupNumber])->inode_bitmap_address;
    int inode_count = groups[groupNumber]->inode_count;
    byte* b = read_block(fd, bitmap_pos, BLOCK_SIZE, error);
    long long count = 1;
    int shift = 0;
    for(int i = 0; count <= inode_count; i++){
        for(int j = 0; j<8 && count <= inode_count; j++){
            if(!(b[i] & (1 << shift))){
                result += "IFREE,"+std::to_string(count)+"\n";
            }
            shift ++;
            count++;
        }
        shift = 0;
    }

    delete[] b;
    return result;
}