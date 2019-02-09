//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "SortedList.h"
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

typedef int bool;
#define false 0
#define true 1

// define several locks to be used in a bitmask
#define MUTEX_LOCK 0x1
#define SPIN_LOCK 0x2
#define SWAP_LOCK 0x4

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct list_arguments{
    int locks;
    SortedList_t* head;
};

char* generate_and_allocate_random_string(int length, int* success){
    static char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    char* value = malloc(sizeof(char)*(length+1));
    if(!value){
        fprintf(stderr, "Unable to allocate memory for key: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        *success = 1;
    }
    for(int i =0; i<length; i++){
        value[i] = alphabet[rand()%strlen(alphabet)];
    }
    value[length] = '\0';
    return value;
}

void add_to_list(SortedList_t* list, SortedListElement_t* element, int lock_flags){
    if(lock_flags & MUTEX_LOCK){
        pthread_mutex_lock(&mutex);
        SortedList_insert(list, element);
        pthread_mutex_unlock(&mutex);
        return;
    }
    if(lock_flags & SWAP_LOCK){
        //TODO
    }
    SortedList_insert(list, element);
}

//TODO: implement this function
void* threaded_function_run(void* args){
    struct list_arguments* a = (struct list_arguments*) args;
    return 0;
}

int opt_yield = 0;

int main(int argc, char** argv){
    srand(time(0)); // set (pseudo)random seed based on time
    int exit_status = 0;
    struct timespec init_time, final_time;
    static struct option long_options[] ={
		{"threads", required_argument, 0, 't'},
		{"iterations", required_argument, 0, 'i'},
		{"yield", required_argument, 0, 'y'},
		{"sync", required_argument, 0, 's'},
		{0,0,0,0}
	};

    int option_index =0;
    int c;
    int thread_count = 1;
    int iteration_count = 1;
    while(true){
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c == -1)
            break;
        switch(c){
            case 't':
                thread_count = atoi(optarg);
                break;
            case 'i':
                iteration_count = atoi(optarg);
                break;
            case 'y':
                opt_yield = 0;
                //parse through the different types of yields that should be utilized
                for(size_t i = 0; i<strlen(optarg); i++){
                    switch(optarg[i]){
                        case 'i':
                            opt_yield = opt_yield | INSERT_YIELD;
                            break;
                        case 'd':
                            opt_yield = opt_yield | DELETE_YIELD;
                            break;
                        case 'l':
                            opt_yield = opt_yield | LOOKUP_YIELD;
                            break;
                        default:
                            fprintf(stderr, "Bad argument to --yield: \"%c\"\n", optarg[i]);
                            exit_status = 1;
                    }
                }
                break;
            case '?':
                exit_status = 1;
                break;
            default:
                exit(2); // something went horribly wrong
        }
    }
    // build up list for iteration_count
    // create the head of the list
    SortedList_t* head = malloc(sizeof(SortedList_t));
    if(!head){
        fprintf(stderr, "Unable to allocate memory for the head of the linked list: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        exit_status = 1;
    }
    head -> key = 0;
    head -> next = 0;
    head -> prev = 0;

    SortedListElement_t** all_elements = malloc(sizeof(SortedListElement_t*)*iteration_count*thread_count);
    if(!all_elements){
        fprintf(stderr, "Unable to allocate memory for the pointers to the elements of the linked list: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        exit_status = 1;
    }
    int success = 0;
    for(int i = 0; i<iteration_count*thread_count; i++){
        all_elements[i] = malloc(sizeof(SortedListElement_t));
        all_elements[i]->key = generate_and_allocate_random_string(10, &success);
        exit_status = exit_status | success;
    }

    // print out
    SortedListElement_t* current = head->next;
    while(current){
        fprintf(stdout, "%s, ", current->key);
        current = current -> next;
    }
    fprintf(stdout, "\n");
    fflush(stdout);

    // Allocate space for threads
    pthread_t* threads = malloc(sizeof(pthread_t)*thread_count);

    // Get clock starting time
    if(clock_gettime(CLOCK_REALTIME,&init_time)){
		fprintf(stderr, "Unable to get inital clock time: %s", strerror(errno));
		exit_status = 1;
	}
    //DO COMPUTATION HERE
    for(int i = 0; i<thread_count; i++){

    }
    // Join threads
    for(int i = 0; i<thread_count; i++){

    }


    if(clock_gettime(CLOCK_REALTIME,&final_time)){
		fprintf(stderr, "Unable to get final clock time: %s", strerror(errno));
		exit_status = 1;
	}

    // delete the whole array of items and free memory associated with keys
    for(int i =0; i<thread_count*iteration_count; i++){
        free((void*)(all_elements[i]->key));
        free(all_elements[i]);
    }
    free(head);
    free(all_elements);
    free(threads);

    return exit_status;
}