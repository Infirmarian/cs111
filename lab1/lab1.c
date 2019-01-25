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
#include <sys/wait.h>

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
    int fd = open(name, filemode, S_IRWXU|S_IRGRP|S_IROTH);
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
int execute_command(int argc, char** argv, int* optind, int_array* arr, proc_array* pida){
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
        fprintf(stdout, "\n");
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
        com_t t;
        t.pid = pid;
        t.arg = argv + *optind + 3;
        t.argc = count - 3;
        add_proc(pida, t); //keep track of the 
    }else{
        // redirect children
        // Child
        redirect_input(arr->array[argv[*optind][0]-'0'], STDIN_FILENO);
        redirect_input(arr->array[argv[*optind+1][0]-'0'], STDOUT_FILENO);
        redirect_input(arr->array[argv[*optind+2][0]-'0'], STDERR_FILENO);

        close_all_fds(arr); // close all other file descriptors in the child

        execvp(argv[*optind+3], argv+*optind+3); //call the argument 
        fprintf(stderr, "Execvp encountered an error: %s", strerror(errno));
        exit(122); //TODO: Fix this
    }
    
    //restore the value in argv
    argv[*optind+count] = stored_arg;
    *optind = *optind + count;
    return 0;
}

// waits for all child processes to complete and reports their exit status
int wait_for_all_pids(proc_array* pida){
    // for each process id
    int maxrc = 0;
    int count = 0;
    pid_t result;
    int status;
    while(count < pida->size){
        result = wait(&status); // waiting here
        if(result == -1){
                fprintf(stderr, "Error waiting for process to complete: %s", strerror(errno));
                fflush(stderr); 
        }
        // get a reference to the child process
        com_t command;
        int found = false;
        for(int i = 0; i<pida->size; i++){
            if(pida->array[i].pid == result){
                command = pida->array[i];
                found = true;
                break;
            }
        }
        if(!found){
            fprintf(stderr, "Child process %d could not be found among spawned processes", result);
            fflush(stderr); 
        }
        if(WIFEXITED(status)){
                maxrc = maxrc > WIFEXITED(status) ? maxrc : WIFEXITED(status);
                // print 
                if(fprintf(stdout, "exit %d ", WIFEXITED(status)) < 0)
                    fprintf(stderr, "Unable to print exit status: %s", strerror(errno));
                for(int j = 0; j<command.argc; j++){
                    if(fprintf(stdout, "%s ", command.arg[j]) < 0)
                        fprintf(stderr, "Unable to print exit status: %s", strerror(errno));
                }
                if(fprintf(stdout, "\n") < 0)
                    fprintf(stderr, "Unable to print newline char: %s", strerror(errno));
                if(fflush(stdout)){
                    fprintf(stderr, "Unable to flush standard output: %s", strerror(errno));
                }
                count ++;
            }else if (WIFSIGNALED(status)){
                maxrc = maxrc > WTERMSIG(status) ? maxrc : WTERMSIG(status);
                if(fprintf(stdout, "signal %d", WTERMSIG(status)) < 0)
                    fprintf(stderr, "Unable to print exit status: %s", strerror(errno));
                for(int j = 0; j<command.argc; j++){
                    if(fprintf(stdout, "%s ", command.arg[j]) < 0)
                        fprintf(stderr, "Unable to print exit status: %s", strerror(errno));
                }
                if(fprintf(stdout, "\n") < 0)
                    fprintf(stderr, "Unable to print newline char: %s", strerror(errno));
                if(fflush(stdout))
                    fprintf(stderr, "Unable to flush standard output: %s", strerror(errno));
                count ++;
            }

    }
    pida->size = 0;
    return maxrc;
}

void signal_handler(int signum){
    fprintf(stderr, "%d caught", signum);
    exit(signum);
}
void ignore_signal(int signum){
    // do nothing here
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
        {"catch", required_argument, 0, 'C'},
        {"ignore", required_argument, 0, 'i'},
        {"default", required_argument, 0, 'd'},
        {"pause", no_argument, 0, 'P'},

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
    proc_array pid_array;
    pid_array.array = malloc(sizeof(com_t)*10);
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
        int rc;
        switch(c){
            case 'p':   // open a pipe
                rc = open_pipe(&fd_array);
                e_acc = rc > e_acc ? rc : e_acc;
                break;
            case 'r':
                if(open_check(optarg, O_RDONLY|fileflag, argv[optind-2], &fd_array) == -1)
                    e_acc = 1 > e_acc ? 1:e_acc;
                fileflag = 0; //reset fileflags
                break;
            case 'w':
                if(open_check(optarg, O_WRONLY|fileflag, argv[optind-2], &fd_array) == -1)
                    e_acc = 1 > e_acc ? 1:e_acc;
                fileflag = 0; //reset fileflags
                break;
            case 'b':
                if(open_check(optarg, O_RDWR|fileflag, argv[optind-2], &fd_array) == -1)
                    e_acc = 1 > e_acc ? 1:e_acc;
                fileflag = 0; //reset fileflags
                break;
            case 'x': // --command (execute)
                rc = execute_command(argc, argv, &optind, &fd_array, &pid_array);
                e_acc = rc > e_acc ? rc : e_acc;
                break;
            //TODO
            case 'a': // --wait
                if(verbose){
                    if(fprintf(stdout, "--wait\n") <0)
                        fprintf(stderr, "Unable to print to stdout");
                    if(fflush(stdout))
                        fprintf(stderr, "Unable to flush standard output");
                }
                rc = wait_for_all_pids(&pid_array);
                e_acc = rc > e_acc ? rc : e_acc;
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
            case 'C': // CATCH N
                if(verbose){
                    if(fprintf(stdout, "--catch %s", optarg) <0)
                        fprintf(stderr, "Unable to print to stdout");
                    if(fflush(stdout))
                        fprintf(stderr, "Unable to flush standard output");

                }
                signal(atoi(optarg), signal_handler);
                break;
            case 'i': //Ignore N
                if(verbose){
                    if(fprintf(stdout, "--ignore %s\n", optarg) <0)
                        fprintf(stderr, "Unable to print to stdout");
                    if(fflush(stdout))
                        fprintf(stderr, "Unable to flush standard output");
                }
                signal(atoi(optarg), ignore_signal);
                break;
            case 'd': //Default case
                if(verbose){
                    if(fprintf(stdout, "--default %s\n", optarg) <0)
                        fprintf(stderr, "Unable to print to stdout");
                    if(fflush(stdout))
                        fprintf(stderr, "Unable to flush standard output");

                }
                signal(atoi(optarg), SIG_DFL);
                break;
            case 'P': // Pause
                if(verbose){
                    if(fprintf(stdout, "--pause\n") <0)
                        fprintf(stderr, "Unable to print to stdout");
                    if(fflush(stdout))
                        fprintf(stderr, "Unable to flush standard output");

                }
                pause();
                break;
            case '?':
                fprintf(stderr, "Error, unknown option %s passed in\n", argv[optind-1]);
                e_acc = 1 > e_acc? 1:e_acc;
                break;
            default:
                // This should never be reached
                exit(1);  
        }
    }
    free(fd_array.array);
    free(pid_array.array);
    return e_acc; //returns accumulated errors
}