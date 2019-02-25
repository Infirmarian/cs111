//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
typedef unsigned char byte;

void Fflush(FILE* f);
int Pread(int fd, void* ptr, size_t size, off_t offset);
byte* read_block(int fd, int block_address, int block_size, int* error);
bool inode_is_free(std::vector<byte> inodes, int number);
bool inode_is_free(byte* b, int number);
bool block_is_free(std::vector<byte> blocks, int number);
bool block_is_free(byte* blocks, int number);
char* convert_to_time(long int total_seconds);
// Inline definition for converting bytes to type
template<class T>
inline T convert_bytes_to_type(byte* b, int pos){
    T i = 0;
    int size = sizeof(T);
    for(int j = 0; j<size; j++){
        i = i | (b[pos+j] << 8*j);
    }
    return i;
}
#endif // MACRO
