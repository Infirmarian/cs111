//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "utils.h"

#define true 1
#define false 0
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

int are_valid_filedescriptors(char** argv, int optind, int_array* arr){
    for(int i = optind; i<optind+3; i++){
        int index = argv[i][0] - '0';
        if(index >= arr->size || arr->array[argv[i][0] - '0'] < 0)
            return false;
    }
    return true;
}

int add_int(int_array * arr, int fd){
    if(arr->max <= arr->size){
        // reallocate memory
        int* temp = realloc(arr->array, sizeof(int)*(arr->max + 10));
        if(!temp)
            fprintf(stderr, "Unable to allocate more room for FD array: %s", strerror(errno));
        arr->max += 10;
        arr->array = temp;
    }
    arr->array[arr->size] = fd;
    arr->size += 1;
    return 0;
}

int add_proc(proc_array* arr, com_t com){
    if(arr->max <= arr->size){
        // reallocate memory
        com_t* temp = realloc(arr->array, sizeof(com_t)*(arr->max + 10));
        if(!temp)
            fprintf(stderr, "Unable to allocate more room for PID array: %s", strerror(errno));
        arr->max += 10;
        arr->array = temp;
    }
    arr->array[arr->size] = com;
    arr->size += 1;
    return 0;
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
// close all file descriptors in an array
int close_all_fds(int_array* arr){
    for(int i = 0; i<arr->size; i++){
        if(arr->array[i] < 0)
            continue;
        if(close(arr->array[i])){
            fprintf(stderr, "Unable to close file descriptor in child process: %s", strerror(errno));
        }
    }
    return 0;
}

void induce_segfault(){
    char* b = 0;
    *b = 10; //assign to a null pointer, cause segmentation fault
}
void safeprint2(char const* str, char const* app, char const* val2){
    if(fprintf(stdout, str, app, val2)<0)
        fprintf(stderr, "Unable to print logging statement: %s", strerror(errno));
    if(fflush(stdout)){
        fprintf(stderr, "Unable to flush stdout: %s", strerror(errno));
        fflush(stderr);
    }
}

void safeprint1(char const* str, char const* app){
    safeprint2(str, app, "");
}

void safeprint(char const* str){
    safeprint1(str, "");
}

void signal_handler(int signum){
    int exitCode = signum;
    int size=1;
    while(signum > 10){
        size*=10;
        signum /=10;
    }
    signum = exitCode;
    char c[1];
    while(size > 0){
        c[0] = (signum/size) +'0';
        write(STDERR_FILENO, c,1);
        signum = signum%size;
        size /=10;
    }
    write(STDERR_FILENO, " caught\n", 8);
    _exit(exitCode);
}

void safegetrusage(int flag, struct rusage * u){
    if(getrusage(flag, u)<0){
        fprintf(stderr, "Unable to get system resources: %s", strerror(errno));
        if(fflush(stderr))
            fprintf(stderr, "Unable to flush standard error: %s", strerror(errno));
    }
}
// THIS FUNCTION ACQUIRED FROM https://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y){
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
void add_times(struct timeval* result, struct timeval* x, struct timeval* y){
    int us = x->tv_usec + y->tv_usec;
    int sec = x->tv_sec + y->tv_sec;
    if(us >= 1000000){
        us -= 1000000;
        sec += 1;
    }
    result->tv_sec = sec;
    result->tv_usec = us;
}


void reportresources(int flag, struct rusage * pre){
    struct rusage post;
    safegetrusage(flag, &post);
    struct timeval userdif;
    struct timeval sysdif;
    struct timeval totalTime;

    if(timeval_subtract(&userdif, &(post.ru_utime), &(pre->ru_utime)))
        fprintf(stderr, "ERROR: Time value was negative\n");
    fprintf(stdout, "user: %ld.%06ld sec, ", userdif.tv_sec, userdif.tv_usec);
    if(timeval_subtract(&sysdif, &(post.ru_stime), &(pre->ru_stime)))
        fprintf(stderr, "ERROR: Time value was negative\n");
    fprintf(stdout, "system: %ld.%06ld sec, ", sysdif.tv_sec, sysdif.tv_usec);
    add_times(&totalTime, &userdif, &sysdif);
    fprintf(stdout, "total: %ld.%06ld sec\n", totalTime.tv_sec, totalTime.tv_usec);
}
