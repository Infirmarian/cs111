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
#include "utils.h"

#define true 1
#define false 0

static int verbose = 0;

// Opens a file safely (eg checks for errors in opening)
int open_check(char* name, int filemode, char* flagname, int_array* fd_array){
    //print the current operation
    if(verbose){
        if(fprintf(stdout, "%s %s\n", flagname, name) < 0)
            fprintf(stderr, "Unable to use fprintf to print command: %s", strerror(errno));

        if(fflush(stdout))
            fprintf(stderr, "Unable to flush stdout: %s", strerror(errno));
    }
    int fd = open(name, filemode);
    add_int(fd_array, fd);
    if(fd < 0){
        fprintf(stderr, "Unable to open file %s: %s\n", name, strerror(errno));
    }
    return fd;
}
// opens a pipe
// returns 0 for a successful pipe and 1 for a failure
int open_pipe(int_array* arr){
    if(verbose){
        if(fprintf(stdout, "--pipe\n") < 0)
            fprintf(stderr, "Unable to use fprintf to print command: %s", strerror(errno));

        if(fflush(stdout))
            fprintf(stderr, "Unable to flush stdout: %s", strerror(errno));
    }
    int pipenames[2] = {0,0};
    int rc = pipe(pipenames);
    if(rc){
        fprintf(stderr, "Unable to create a pipe: %s\n", strerror(errno));
    }
    // add both read and write ends of the pipe
    add_int(arr, pipenames[0]);
    add_int(arr, pipenames[1]);
    return !!rc;

}
// close a file descriptor
int close_fd(int_array* fd_array, int index){
    int rc = 0;
    if(close(fd_array->array[index])){
        fprintf(stderr, "Unable to close file index %d: %s", index, strerror(errno));
        fflush(stderr);
        rc = 1;
    }
    fd_array->array[index] = -1;
    return rc;
}

// executes a command 
int execute_command(int argc, char** argv, int* optind, int_array* arr, int_array* pida){
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
    // too few arguments passed in, eg no command
    if(count < 4){
        fprintf(stderr, "Command provided too few arguments, unable to execute");
        return 1;
    }
    if(! are_valid_filedescriptors(argv, *optind, arr)){
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
        add_int(pida, pid); //keep track of the 
    }else{
        // redirect children
        // Child
        redirect_input(arr->array[argv[*optind][0]-'0'], STDIN_FILENO);
        redirect_input(arr->array[argv[*optind+1][0]-'0'], STDOUT_FILENO);
        redirect_input(arr->array[argv[*optind+2][0]-'0'], STDERR_FILENO);

        execvp(argv[*optind+3], argv+*optind+3); //call the argument 
        
    }
    
    //restore the value in argv
    argv[*optind+count] = stored_arg;
    *optind = *optind + count;
    return 0;
}

// waits for all child processes to complete and reports their exit status
int wait_for_all_pids(int_array* pida){
    // for each process id
    for(int i = 0; i<pida->size; i++){
        int status;
        //TODO: Implement waiting for child processes to finish
    }
    return 0;
}

int main(int argc, char** argv){
    int option_index = 0;
    static int flags[11] = {
        O_APPEND, O_CLOEXEC, O_CREAT, O_DIRECTORY, O_DSYNC, O_EXCL, O_NOFOLLOW, O_NONBLOCK,  O_RSYNC, O_SYNC, O_TRUNC
    };
    static struct option long_options[] = {
        {"rdonly", required_argument, 0, 'r'},
        {"wronly", required_argument, 0, 'w'},
        {"rdwr", required_argument, 0, 'b'},
        {"pipe", no_argument, 0, 'p'},
        // file flags
        {"append", no_argument, 0, 0},
        {"cloexec", no_argument, 0, 1},
        {"creat", no_argument, 0, 2},
        {"directory", no_argument, 0, 3},
        {"dsync", no_argument, 0, 4},
        {"excl", no_argument, 0, 5},
        {"nofollow", no_argument, 0, 6},
        {"nonblock", no_argument, 0, 7},
        {"rsync", no_argument, 0, 8},
        {"sync", no_argument, 0, 9},
        {"trunc", no_argument, 0, 10},

        {"command", required_argument, 0, 'x'},
        {"wait", no_argument, 0, 'a'},

        //extra options
        {"close", required_argument, 0, 'c'},
        {"abort", no_argument, 0, 'z'},
        {"verbose", no_argument, 0, 'v'},
        {0,0,0,0}
    };
    int e_acc = 0;
    int c;

    // set up array for file descriptors
    int_array fd_array;
    fd_array.array = malloc(sizeof(int)*10);
    if(!fd_array.array)
            fprintf(stderr, "Unable to allocate an initial file descriptor array: %s", strerror(errno));
    fd_array.size = 0;
    fd_array.max = 10;
    // Setup array for Process IDs
    int_array pid_array;
    pid_array.array = malloc(sizeof(int)*10);
    if(!pid_array.array)
            fprintf(stderr, "Unable to allocate an initial PID array: %s", strerror(errno));
    pid_array.size = 0;
    pid_array.max = 10;


    int fileflag = 0;
    //loop through and parse options
    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1)
            break;
        // sets up the fileflags
        if(c<11){
            if(verbose){
                if(fprintf(stdout, "--%s\n", long_options[option_index].name)<0){
                    fprintf(stderr, "Unable to print verbose logging statement: %s", strerror(errno));
                }
                if(fflush(stdout)){
                    fprintf(stderr, "Unable to flush stdout: %s", strerror(errno));
                }
            }
            fileflag = fileflag | flags[c];
            continue;
        }
        switch(c){
            case 'p':   // open a pipe
                e_acc += open_pipe(&fd_array);
                break;
            case 'r':
                if(open_check(optarg, O_RDONLY|fileflag, argv[optind-2], &fd_array) == -1)
                    e_acc ++;
                fileflag = 0; //reset fileflags
                break;
            case 'w':
                if(open_check(optarg, O_WRONLY|fileflag, argv[optind-2], &fd_array) == -1)
                    e_acc ++;
                fileflag = 0; //reset fileflags
                break;
            case 'b':
                if(open_check(optarg, O_RDWR|fileflag, argv[optind-2], &fd_array) == -1)
                    e_acc ++;
                fileflag = 0; //reset fileflags
                break;
            case 'x': // --command (execute)
                e_acc += execute_command(argc, argv, &optind, &fd_array, &pid_array);
                break;
            //TODO
            case 'a': // --wait
                
                break;
            case 'z':
                induce_segfault(verbose);
                break;
            case 'c':
                close_fd(&fd_array, optarg[0]-'0');
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                fprintf(stderr, "Error, unknown option %s passed in\n", argv[optind-1]);
                e_acc += 1;
                break;
            default:
                // This should never be reached
                exit(1);  
        }
    }
    free(fd_array.array);
    free(pid_array.array);
    return !!e_acc; //returns accumulated errors
}