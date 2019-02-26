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


std::string formatSuperData(int fd, int& error);
std::string formatGroupData(int fd, int groupNumber, int& error);
std::string getFreeBlockData(int fd, int groupNumber, int& error);
std::string getFreeInodeData(int fd, int groupNumber, int& error);
bool allInodes(int fd, int groupNumber, int& error);

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
    fprintf(stdout, "%s\n", formatSuperData(img_fd, e).c_str()); //SUPERBLOCK
    fprintf(stdout, "%s\n", formatGroupData(img_fd, 0, e).c_str()); // GROUP DATA
    fprintf(stdout, "%s", getFreeBlockData(img_fd, 0, e).c_str()); // Block DATA
    fprintf(stdout, "%s", getFreeInodeData(img_fd, 0, e).c_str()); // Inode DATA
    allInodes(img_fd, 0, e); // Data is printed within, to prevent a core dump due to "stack smashing"
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

struct ext2_group_desc* getGroupData(int fd, int groupNumber, int& error){
    byte* b = read_block(fd, SUPERBLOCK_LOCATION + 1, BLOCK_SIZE, error);
    struct ext2_group_desc* groupData = read_bytes_into_struct<struct ext2_group_desc>(b,groupNumber*32);
    groups.insert(std::pair<int, struct ext2_group_desc*>(groupNumber, groupData));
    delete[] b;
    return groupData;
}
struct ext2_super_block* getSuperblockData(int fd, int& error){
    byte* b = read_block(fd, SUPERBLOCK_LOCATION, BLOCK_SIZE, error);
    struct ext2_super_block* superblock = read_bytes_into_struct<struct ext2_super_block>(b, 0);
    superblockdata = superblock;
    delete[] b;
    return superblock;
}
std::string formatSuperData(int fd, int& error){
    // Located 1024 bytes from the beginning of the filesystem
    std::string result = "SUPERBLOCK,";
    if(!superblockdata){
        getSuperblockData(fd, error);
    }
    // Block size needs to be 1024 shifted by block size
    BLOCK_SIZE = EXT2_MIN_BLOCK_SIZE << superblockdata->s_log_block_size;

    result  +=  std::to_string(superblockdata->s_blocks_count)+","+std::to_string(superblockdata->s_inodes_count)+
                ","+std::to_string(BLOCK_SIZE)+ ","+std::to_string(superblockdata->s_inode_size) + "," + 
                std::to_string(superblockdata->s_blocks_per_group) + "," + std::to_string(superblockdata->s_inodes_per_group) + 
                "," + std::to_string(superblockdata->s_first_ino);
    return result;
}


std::string formatGroupData(int fd, int groupNumber, int& error){
    std::string result = "GROUP,"; // Guaranteed to only have 1 Group
    std::unordered_map<int, struct ext2_group_desc*>::iterator it = groups.find(groupNumber);
    if(it == groups.end()){
        getGroupData(fd, groupNumber, error);
        it = groups.find(groupNumber);
    }
    struct ext2_group_desc* gd = it->second;
    // Gather information from the superblock, if it hasn't already been done
    if(!superblockdata){
        getSuperblockData(fd, error);
    }

    int block_count = superblockdata->s_blocks_count;
    block_count = superblockdata->s_blocks_count % superblockdata->s_blocks_per_group; // only for the last group
    
    result += std::to_string(groupNumber)+","+std::to_string(block_count)+","+std::to_string(superblockdata->s_inodes_per_group)+
            ","+std::to_string(gd->bg_free_blocks_count)+","+ std::to_string(gd->bg_free_inodes_count) + 
            ","+std::to_string(gd->bg_block_bitmap) + ","+std::to_string(gd->bg_inode_bitmap) + 
            ","+ std::to_string(gd->bg_inode_table);
    return result;
}

// Return a string that represents each free block in the given group
std::string getFreeBlockData(int fd, int groupNumber, int& error){
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
std::string getFreeInodeData(int fd, int groupNumber, int& error){
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

struct ext2_inode* getInodeEntry(byte* block, int number){
    struct ext2_inode* s = read_bytes_into_struct<ext2_inode>(block, number*INODE_TABLE_OFFSET);
    return s;
}

// Returns the type of this file
char formatAndPrintInodeSummary(byte* block, int inode_number){
    std::string result = "INODE,";
    inode_number --;
    struct ext2_inode* s = getInodeEntry(block, inode_number);
    if(s->i_mode == 0 || s->i_links_count == 0){
        return ' ';
    }
    unsigned short usermode = s->i_mode & 0x0FFF;
    char filetype;
    // get char representation of file type
    switch(s->i_mode & 0xF000){
        case 0x8000:
        filetype = 'f';
        break;
        case 0x4000:
        filetype = 'd';
        break;
        case 0xA000:
        filetype = 's';
        break;
        default:
        filetype = '?';
        break;
    }
    char userModeChars[4];
    sprintf(userModeChars, "%o", usermode);

    std::string c_time = convert_to_time(s->i_ctime);
    std::string m_time = convert_to_time(s->i_mtime);
    std::string a_time = convert_to_time(s->i_atime);

    std::string addressblocks = "";
    if(s->i_size > 60 || filetype != 's'){
        for(int i = 0; i<EXT2_N_BLOCKS; i++){
            addressblocks += std::to_string(s->i_block[i])+",";
        }
        addressblocks = addressblocks.substr(0, addressblocks.length()-1);
    }

    result += std::to_string(inode_number + 1)+","+filetype+",";
    result.append(userModeChars);
    result += ","+std::to_string(s->i_uid)+","+std::to_string(s->i_gid)+","+std::to_string(s->i_links_count)+
    ","+c_time+","+m_time+","+a_time+","+std::to_string(s->i_size)+","+std::to_string(s->i_blocks)+","+addressblocks;

    fprintf(stdout, "%s\n", result.c_str());
    return filetype;
}

int printDirectory(byte* block, int inode_number, int offset){
    struct ext2_dir_entry* ddata = read_bytes_into_struct<ext2_dir_entry>(block, offset);
    if(ddata->inode == 0){
        return offset + ddata->rec_len;
    }
    fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,", inode_number, offset, ddata->inode, ddata->rec_len, ddata->name_len);
    std::string name = "";
    for(int i = 0; i<ddata->name_len; i++){
        name += ddata-> name[i];
    }
    fprintf(stdout, "'%s'\n", name.c_str());
    return offset + ddata->rec_len;
}
void evaluateDirectoryBlock(int fd, int block_addr, int inode_number, int& error){
    if(block_addr == 0)
        return;
    int offset = 0;
    byte* b = read_block(fd, block_addr, BLOCK_SIZE, error);
    while(offset < BLOCK_SIZE){
        offset = printDirectory(b, inode_number + 1, offset);
    }
    delete[] b;
}


void formatAndPrintDirectSummary(int fd, byte* inode_block, int inode_number, int& error){
    inode_number --;
    struct ext2_inode* s = getInodeEntry(inode_block, inode_number);
    // Direct blocks
    for(int i = 0; i<EXT2_NDIR_BLOCKS; i++){
        evaluateDirectoryBlock(fd, s->i_block[i], inode_number, error);
    }
    // Indirect blocks
    if(s->i_block[EXT2_IND_BLOCK]){
        byte* b = read_block(fd, s->i_block[EXT2_IND_BLOCK], BLOCK_SIZE, error);
        for(int i = 0; i<BLOCK_SIZE; i+=4){
            int block = convert_bytes_to_type<int>(b, i);
            evaluateDirectoryBlock(fd, block, inode_number, error);
        }
        delete[] b;
    }
    // Doubly Indirect blocks
    if(s->i_block[EXT2_DIND_BLOCK]){
        byte* b = read_block(fd, s->i_block[EXT2_DIND_BLOCK], BLOCK_SIZE, error);
        for(int i = 0; i<BLOCK_SIZE; i+=4){
            int diblock = convert_bytes_to_type<int>(b, i); // first indirect block
            if(diblock){
                byte* db = read_block(fd, diblock, BLOCK_SIZE, error);
                for(int j = 0; j<BLOCK_SIZE; j+=4){
                    int block = convert_bytes_to_type<int>(db, j);
                    evaluateDirectoryBlock(fd, block, inode_number, error);
                }
                delete[] db;
            }
        }
        delete[] b;
    }
    // Triply Indirect blocks
    if(s->i_block[EXT2_TIND_BLOCK]){
        byte* b = read_block(fd, s->i_block[EXT2_DIND_BLOCK], BLOCK_SIZE, error);
        for(int i = 0; i<BLOCK_SIZE; i+=4){
            int diblock = convert_bytes_to_type<int>(b, i); // first indirect block
            if(diblock){
                byte* db = read_block(fd, diblock, BLOCK_SIZE, error);
                for(int j = 0; j<BLOCK_SIZE; j+=4){
                    int tiblock = convert_bytes_to_type<int>(db, j);
                    if(tiblock){
                        byte* tb = read_block(fd, tiblock, BLOCK_SIZE, error);
                        for(int k = 0; k<BLOCK_SIZE; k+=4){
                            int block = convert_bytes_to_type<int>(tb, k);
                            evaluateDirectoryBlock(fd, block, inode_number, error);
                        }
                        delete[] tb;
                    }
                }
                delete[] db;
            }
        }
        delete[] b;
    }
}


bool allInodes(int fd, int groupNumber, int& error){
    std::unordered_map<int, struct ext2_group_desc*>::iterator it = groups.find(groupNumber);
    if(it == groups.end()){
        getGroupData(fd, groupNumber, error);
        it = groups.find(groupNumber);
    }
    int bitmap_pos = it->second->bg_inode_bitmap;
    int inode_count = superblockdata->s_inodes_per_group;
    int inode_table_address = it->second->bg_inode_table;
    byte* b = read_block(fd, bitmap_pos, BLOCK_SIZE, error);
    int blocks_to_read = (1+(INODE_TABLE_OFFSET*inode_count)/BLOCK_SIZE);
    byte* inodes = read_blocks(fd, inode_table_address, BLOCK_SIZE, blocks_to_read,error);
    for(int i = 1; i <= inode_count; i++){
        if(!inode_is_free(b, i)){
            char type = formatAndPrintInodeSummary(inodes, i);
            if(type == 'd'){
                formatAndPrintDirectSummary(fd, inodes, i, error);
            }
        }
    }
    delete[] b;
    delete[] inodes;
    return true;
}