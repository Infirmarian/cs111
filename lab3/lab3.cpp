//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <vector>
#include <unordered_map>

#include "utils.h"
#include "ext2_fs.h"

#define SUPERBLOCK_LOCATION 1
#define INODE_TABLE_OFFSET 128

static int BLOCK_SIZE = 1024;
static struct ext2_super_block* superblockdata = 0;


std::unordered_map<int, struct ext2_group_desc*> groups;
std::unordered_map<int, std::vector<byte>> groupInodeData;

std::string getSuperblockData(int fd, int* error){
    // Located 1024 bytes from the beginning of the filesystem
    std::string result = "SUPERBLOCK,";
    byte* b = read_block(fd, SUPERBLOCK_LOCATION, BLOCK_SIZE, error);
    struct ext2_super_block* superblock = read_bytes_into_struct<struct ext2_super_block>(b,0);
    superblockdata = superblock;
    // Block size needs to be 1024 shifted by block size
    BLOCK_SIZE = EXT2_MIN_BLOCK_SIZE << superblock->s_log_block_size;

    result  +=  std::to_string(superblock->s_blocks_count)+","+std::to_string(superblock->s_inodes_count)+
                ","+std::to_string(BLOCK_SIZE)+ ","+std::to_string(superblock->s_inode_size) + "," + 
                std::to_string(superblock->s_blocks_per_group) + "," + std::to_string(superblock->s_inodes_per_group) + 
                "," + std::to_string(superblock->s_first_ino);
    delete[] b;
    // don't delete the superblock data
    return result;
}

std::string getGroupData(int fd, int groupNumber, int* error){
    std::string result = "GROUP,"; // Guaranteed to only have 1 Group
    byte* b = read_block(fd, SUPERBLOCK_LOCATION + 1, BLOCK_SIZE, error);
    struct ext2_group_desc* groupData = read_bytes_into_struct<struct ext2_group_desc>(b,groupNumber*32);

    // Gather information from the superblock, if it hasn't already been done
    if(!superblockdata){
        getSuperblockData(fd, error);
    }
    int block_count = superblockdata->s_blocks_count;
    block_count = superblockdata->s_blocks_count % superblockdata->s_blocks_per_group; // only for the last group

    result += std::to_string(groupNumber)+","+std::to_string(block_count)+","+std::to_string(superblockdata->s_inodes_per_group)+
            ","+std::to_string(groupData->bg_free_blocks_count)+","+ std::to_string(groupData->bg_free_inodes_count) + 
            ","+std::to_string(groupData->bg_block_bitmap) + ","+std::to_string(groupData->bg_inode_bitmap) + 
            ","+ std::to_string(groupData->bg_inode_table);
    
    groups.insert(std::pair<int, struct ext2_group_desc*>(groupNumber, groupData));
    delete[] b;
    return result;
}

std::string getFreeBlockData(int fd, int groupNumber, int* error);
std::string getFreeInodeData(int fd, int groupNumber, int* error);
bool allInodes(int fd, int groupNumber, int* error);

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
    allInodes(img_fd, 0, &e); // Data is printed within, to prevent a core dump due to "stack smashing"
    if(e > 0){
        exit_status = 1;
    }

    if(close(img_fd)){
        fprintf(stderr, "Unable to close disk image file: %s\n", strerror(errno));
        Fflush(stderr);
        exit_status = 1;
    }
    if(superblockdata){
        free(superblockdata);
    }
    for(std::unordered_map<int, struct ext2_group_desc*>::iterator it = groups.begin(); it!=groups.end(); it++){
        delete it->second;
    }
    return exit_status;
}

// Return a string that represents each free block in the given group
std::string getFreeBlockData(int fd, int groupNumber, int* error){
    std::string result = "";
    std::unordered_map<int, struct ext2_group_desc*>::iterator it = groups.find(groupNumber);
    if(it == groups.end()){
        getGroupData(fd, groupNumber, error);
        groups.find(groupNumber);
    }
    byte* b = read_block(fd, it->second->bg_block_bitmap, BLOCK_SIZE, error);
    int block_count = superblockdata->s_blocks_per_group;
    for(int i = 1; i <= block_count; i++){
        if(block_is_free(b, i)){
            result += "BFREE,"+std::to_string(i)+"\n";
        }
    }

    delete[] b;
    return result;
}

// Return a string that represents each free block in the given group
std::string getFreeInodeData(int fd, int groupNumber, int* error){
    std::string result = "";
    std::unordered_map<int, struct ext2_group_desc*>::iterator it = groups.find(groupNumber);
    if(it == groups.end()){
        getGroupData(fd, groupNumber, error);
        it = groups.find(groupNumber);
    }
    int inode_count = superblockdata->s_inodes_per_group;
    byte* b = read_block(fd, it->second->bg_inode_bitmap, BLOCK_SIZE, error);

    for(int i = 1; i<=inode_count; i++){
        if(inode_is_free(b, i)){
            result += "IFREE,"+std::to_string(i)+"\n";
        }
    }

    delete[] b;
    return result;
}

void inodeSummary(byte* block, int inode_number){
    std::string result = "INODE,";
    int offset = inode_number * INODE_TABLE_OFFSET;
    unsigned short imode = convert_bytes_to_type<unsigned short>(block, offset+0);
    if(!imode){
        return;
    }
    unsigned short usermode = imode & 0x0FFF;
    char filetype;
    bool print_block_addresses = true;
    switch(imode & 0xF000){
        case 0x8000:
        filetype = 'f';
        break;
        case 0x4000:
        filetype = 'd';
        break;
        case 0xA000:
        filetype = 's';
        print_block_addresses = false;
        break;
        default:
        filetype = '?';
        break;
    }
    char buf[5];
    sprintf(buf, "%o", usermode);

    unsigned short owner = convert_bytes_to_type<unsigned short>(block, offset + 2);
    unsigned short group = convert_bytes_to_type<unsigned short>(block, offset + 24);
    unsigned short link_count = convert_bytes_to_type<unsigned short>(block, offset + 26);

    unsigned int creation_time = convert_bytes_to_type<unsigned int>(block, offset + 12);
    unsigned int modification_time = convert_bytes_to_type<unsigned int>(block, offset + 16);
    unsigned int access_time = convert_bytes_to_type<unsigned int>(block, offset + 8);
    char* c_time = convert_to_time(creation_time);
    char* m_time = convert_to_time(modification_time);
    char* a_time = convert_to_time(access_time);

    unsigned int file_size = convert_bytes_to_type<unsigned int>(block, offset + 4);
    if(file_size > 60){
        print_block_addresses = true;
    }
    unsigned int blocks_used = convert_bytes_to_type<unsigned int>(block, offset + 28);

    // Get the direct, indirect, double and triple indirect values for blocks pointed to
    std::string block_string = "";
    int baddress;
    bool done = print_block_addresses;
    for(int i = 0; i<16; i++){
        if(done){
            block_string += "0,";
            continue;
        }
        baddress = convert_bytes_to_type<unsigned int>(block, offset + 40 + i*4);
        if(baddress){
            block_string += std::to_string(baddress)+",";
        }else{
            done = true;
            i--;
        }
    }
    block_string = block_string.substr(0, block_string.length()-1);

    fprintf(stdout, "INODE,%d,%c,%s,%d,%d,%d,%s,%s,%s,%d,%d", inode_number, filetype, buf, owner, group, link_count,
            c_time, m_time, a_time,
            file_size, blocks_used);
    fprintf(stdout, "%s\n", block_string.c_str());
    free(c_time);
    free(a_time);
    free(m_time);
}

bool allInodes(int fd, int groupNumber, int* error){
    std::unordered_map<int, struct ext2_group_desc*>::iterator it = groups.find(groupNumber);
    if(it == groups.end()){
        getGroupData(fd, groupNumber, error);
        it = groups.find(groupNumber);
    }
    int bitmap_pos = it->second->bg_inode_bitmap;
    int inode_count = superblockdata->s_inodes_per_group;
    int inode_table_address = it->second->bg_inode_table;
    std::string result = "";
    byte* b = read_block(fd, bitmap_pos, BLOCK_SIZE, error);
    byte* inodes = read_block(fd, inode_table_address, BLOCK_SIZE, error);
    for(int i = 1; i <= inode_count; i++){
        if(!inode_is_free(b, i)){
            inodeSummary(inodes, i); // Stuff is printed within inodeSummary, because .c_str is a bad function :/
        }
    }
    return true;
}