//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
typedef unsigned char byte;

void Fflush(FILE* f);
int Pread(int fd, void* ptr, size_t size, off_t offset);
byte* read_block(int fd, int block_address, int block_size, int* error);
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
