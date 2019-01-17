//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

int openrdonly(char* name){
    int fd = open(name, O_RDONLY);
    if(fd < 0){
        fprintf(stderr, "Unable to open file %s: %s", name, strerror(errno));
    }
    return fd;
}

int main(int argc, char** argv){
    int option_index = 0;
    static struct option long_options[] = {
        {"rdonly", required_argument, 0, 'r'},
        {"wronly", required_argument, 0, 'w'},
        {"command", required_argument, 0, 'c'},
        {"verbose", no_argument, 0, 'v'},
        {0,0,0,0}
    };
    int c;
    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1)
            break;
        switch(c){
            case 'r':
                printf("rdonly passed with arg %s\n", optarg);
                break;
            case 'w':
                printf("wronly passed with arg %s\n", optarg);
                break;
            case 'c':
                printf("command passed with arg(s) %s\n", optarg);
                break;
            case '?':
                printf("Unknown arg passed in\n");
                break;
            default:
                exit(1);  
        }
    }
}