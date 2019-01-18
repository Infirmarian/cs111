//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

static int verbose = 0;

int open_check(char* name, int filemode, char* flagname){
    //print the current operation
    if(verbose){
        fprintf(stdout, "%s %s\n", flagname, name);
        fflush(stdout);
    }
    int fd = open(name, filemode);
    if(fd < 0){
        fprintf(stderr, "Unable to open file %s: %s\n", name, strerror(errno));
        return -1;
    }
    return fd;
}
// given a max argument count, an array of strings and an index, the number of arguments
// until the next argument beginning with '--' is returned
int get_argument_count(int argc, char** argv, int optind){
    int count = 0;
    while(optind<argc){
        char* cmd = argv[optind];
        if(strlen(cmd) >=2 && cmd[0] == '-' && cmd[1] == '-')
            break;
        optind ++;
        count ++;
    }
    return count;
}

int execute_command(int argc, char** argv, int* optind){
    (*optind)--;
    int count = get_argument_count(argc, argv, *optind);
    char* stored_arg = 0;
    if(*optind+count < argc){
        stored_arg = argv[*optind+count];
        argv[*optind+count] = 0; //set to the null pointer
    }
    //TODO: Execute the command here
    pid_t pid;
    pid = fork();
    if(pid < 0){
        fprintf(stderr, "Unable to fork to child process: %s", strerror(errno));
        return 1;
    }
    if(pid)
        printf("Parent\n");
    else{
        printf("I'm a child\n");
        exit(execvp(argv[*optind+3], argv+*optind+4));
    }
    do{

    }while(wait(&pid));


    //restore the value in argv
    argv[*optind+count] = stored_arg;
    *optind = *optind + count;
    return 0;
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
    int e_acc = 0;
    int c;
    //loop through and parse options
    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1)
            break;
        switch(c){
            case 'r':
                e_acc += open_check(optarg, O_RDONLY, argv[optind-2]) == -1 ? 1:0;
                break;
            case 'w':
                e_acc += open_check(optarg, O_WRONLY, argv[optind-2]) == -1 ? 1:0;
                break;
            case 'c':
                e_acc += execute_command(argc, argv, &optind);
                break;
            case 'v':
                printf("Verbose mode on\n");
                verbose = 1;
                break;
            case '?':
                printf("Unknown arg passed in\n");
                break;
            default:
                exit(1);  
        }
    }
    return e_acc; //returns accumulated errors
}