//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

std::string getSuperblockData(int fd){
    std::string result = "SUPERBLOCK,";
    return result;
}
// Utility to cleanly flush and check an output
void Fflush(FILE* f){
    if(fflush(f)){
        fprintf(stderr, "Unable to flush to file: %s\n", strerror(errno));
    }
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

    // Produce the series of output required by the program
    fprintf(stdout, "%s\n", getSuperblockData(img_fd).c_str());

    if(close(img_fd)){
        fprintf(stderr, "Unable to close disk image file: %s\n", strerror(errno));
        Fflush(stderr);
        exit_status = 1;
    }
    return exit_status;
}