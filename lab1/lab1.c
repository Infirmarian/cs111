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

#define true 1
#define false 0

static int verbose = 0;

int are_valid_filedescriptors(char** argv, int optind, int fd_count, int* fd_array){
    for(int i = optind; i<optind+3; i++){
        int index = argv[i][0] - '0';
        if(index >= fd_count || fd_array[argv[i][0] - '0'] < 0)
            return false;
    }
    return true;
}
//TODO: add the case in which a pipe is created, consuming 2 fds
int add_filedescriptor(int * arr, int fd, int* size, int* maxsize){
    if(*maxsize == *size){
        // reallocate memory
        if(! realloc(arr, sizeof(int)*(*maxsize + 10)))
            fprintf(stderr, "Unable to allocate more room for file descriptors: %s", strerror(errno));
        *maxsize = *maxsize + 10;
    }
    arr[*size] = fd;
    *size = *size + 1;
    return 0;
}

int open_check(char* name, int filemode, char* flagname, int* fd_array, int* size, int* maxsize){
    //print the current operation
    if(verbose){
        if(fprintf(stdout, "%s %s\n", flagname, name) < 0)
            fprintf(stderr, "Unable to use fprintf to print command: %s", strerror(errno));

        if(fflush(stdout))
            fprintf(stderr, "Unable to flush stdout: %s", strerror(errno));
    }
    int fd = open(name, filemode);
    if(fd < 0){
        fprintf(stderr, "Unable to open file %s: %s\n", name, strerror(errno));
    }
    add_filedescriptor(fd_array, fd, size, maxsize);
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

int redirect_input(int oldfd, int newfd){
    int nfd = dup2(oldfd, newfd);
    if(nfd<0){
        fprintf(stderr, "Unable to redirect input: %s\n", strerror(errno));
        if(fflush(stderr))
            fprintf(stderr, "Unable to flush stderr: %s", strerror(errno));
        return -1;
    }
    return 0;
}

int execute_command(int argc, char** argv, int* optind, int fd_count, int* fd_array){
    (*optind)--;
    int count = get_argument_count(argc, argv, *optind); // get the number of arguments for command

    // if verbose, print out the function called
    if(verbose){
        int i = (*optind) - 1;
        while(i<(*optind)+count){
                if(fprintf(stdout, "%s ", argv[i++])<0){
                    fprintf(stderr, "Error occurred while printing verbose command: %s", strerror(errno));
                }
        }
        if(fflush(stdout)){
            fprintf(stderr, "Error occurred while flushing stdout: %s", strerror(errno));
        }
    }

    if(! are_valid_filedescriptors(argv, *optind, fd_count,fd_array)){
        fprintf(stderr, "Invalid file descriptors provided to command\n");
        if(fflush(stderr)){
            fprintf(stderr, "Error occurred while flushing stderr: %s", strerror(errno));
        }
        // set optind to be the next spot in the argv index
        *optind = *optind + count;
        return 1;
    }
    char* stored_arg = 0;
    if(*optind+count < argc){
        stored_arg = argv[*optind+count];
        argv[*optind+count] = 0; //set to the null pointer
    }
    
    pid_t pid;
    pid = fork();
    if(pid < 0){
        fprintf(stderr, "Unable to fork to child process: %s", strerror(errno));
        fflush(stderr);
        return 1;
    }
    if(pid){
        // Parent
    }else{
        // redirect children
        // Child
        redirect_input(fd_array[argv[*optind][0]-'0'], STDIN_FILENO);
        redirect_input(fd_array[argv[*optind+1][0]-'0'], STDOUT_FILENO);
        redirect_input(fd_array[argv[*optind+2][0]-'0'], STDERR_FILENO);

        execvp(argv[*optind+3], argv+*optind+3); //call the argument 
        
    }
    
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
    // set up array for file descriptors
    int* fd_array = malloc(sizeof(int)*10);
    if(!fd_array)
        fprintf(stderr, "Unable to allocate an initial file descriptor array: %s", strerror(errno));
    int fd_count = 0;
    int fd_array_size = 10;
    //loop through and parse options
    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1)
            break;
        switch(c){
            case 'r':
                if(open_check(optarg, O_RDONLY, argv[optind-2], fd_array, &fd_count, &fd_array_size) == -1){
                    e_acc ++;
                }
                break;
            case 'w':
                if(open_check(optarg, O_WRONLY, argv[optind-2], fd_array, &fd_count, &fd_array_size) == -1){
                    e_acc ++;
                }
                break;
            case 'c':
                e_acc += execute_command(argc, argv, &optind, fd_count, fd_array);
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                fprintf(stderr, "Error, unknown option %s passed in\n", argv[optind-1]);
                e_acc += 1;
                break;
            default:
                exit(1);  
        }
    }
    return !!e_acc; //returns accumulated errors
}