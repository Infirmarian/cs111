//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "SortedList.h"
#include "utils.h"

typedef int bool;
#define false 0
#define true 1

// define several locks to be used in a bitmask
#define MUTEX_LOCK 0x1
#define SPIN_LOCK 0x2

int opt_yield = 0;

// Declare a struct to hold both a list and certain locks for that list
struct ll{
    pthread_mutex_t mutex;
    volatile int spin_lock;
    SortedList_t* list;
};
typedef struct ll locked_list;

// Arguments that are passed to each thread
struct list_arguments{
    int locks;
    locked_list** lists;
    int list_count;
    int count;
    SortedListElement_t** elements;
    long long wait_time;
};

static struct option long_options[] ={
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", required_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {"lists", required_argument, 0, 'l'},
    {0,0,0,0}
};

void* threaded_function_run(void* args);

void signal_handle(int sig){
    write(STDERR_FILENO, "Segmentation Fault, likely due to a corrupted list. Exiting\n", 61);
    (void)sig; // suppress compiler warning about variable
    _exit(2);
}

int main(int argc, char** argv){
    srand(time(0)); // set (pseudo)random seed based on time
    int exit_status = 0;
    // set up signal handler
    if(SIG_ERR == signal(SIGSEGV, signal_handle)){ 
        fprintf(stderr, "Error setting signal handler: %s", strerror(errno));
        exit_status = 1;
    }
    struct timespec init_time, final_time;
    // Input variables for the 
    int thread_count = 1;
    int iteration_count = 1;
    int list_count = 1;

    int option_index =0;
    int c;
    char program_name[16] = "list-";
    int sync_type = 0;
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
            case 'l':
                list_count = atoi(optarg);
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
            case 's':
                switch(optarg[0]){
                    case 's':
                        sync_type = SPIN_LOCK;
                        break;
                    case 'm':
                        sync_type = MUTEX_LOCK;
                        break;
                    default:
                        fprintf(stderr, "Bad argument to --sync: \"%c\"\n", optarg[0]);
                        exit_status = 1;
                }
                break;
            case '?':
                exit_status = 1;
                break;
            default:
                exit(2); // something went horribly wrong
        }
    }
    // set up string for print out value (This is long and pretty tedious)
    if(opt_yield){
        int l = strlen(program_name);
        if(opt_yield & INSERT_YIELD){
            program_name[l] = 'i';
            l++;
        }
        if(opt_yield & DELETE_YIELD){
            program_name[l] = 'd';
            l++;
        }
        if(opt_yield & LOOKUP_YIELD){
            program_name[l] = 'l';
            l++;
        }
        program_name[l] = 0;
    }else{
        int l = strlen(program_name);
        program_name[l++]='n';
        program_name[l++]='o';
        program_name[l++]='n';
        program_name[l++]='e';
        program_name[l++]='\0';
    }
    int l = strlen(program_name);
    program_name[l++] = '-';
    if(sync_type){
        if(sync_type & MUTEX_LOCK){
            program_name[l++] = 'm';
            program_name[l] = 0;
        }
        if(sync_type & SPIN_LOCK){
            program_name[l++] = 's';
            program_name[l] = 0;
        }
    }else{
        int l = strlen(program_name);
        program_name[l++]='n';
        program_name[l++]='o';
        program_name[l++]='n';
        program_name[l++]='e';
        program_name[l++]='\0';
    }

    // build up list for iteration_count
    // create the head of the list
    locked_list ** list_list = malloc(sizeof(locked_list*)*list_count);
    if(!list_list){
        fprintf(stderr, "Unable to allocate memory for the list of locked lists: %s\n", strerror(errno));
        FFlush(stderr);
        exit_status = 1;
    }
    for(int i = 0; i<list_count; i++){
        list_list[i] = malloc(sizeof(locked_list));
        SortedList_t* list_spot = malloc(sizeof(SortedList_t));
        if(!list_list[i] || !list_spot){
            fprintf(stderr, "Unable to allocate memory for one of the lists: %s", strerror(errno));
            FFlush(stderr);
            exit_status = 1;
        }
        list_spot -> key = 0;
        list_spot -> next = 0;
        list_spot -> prev = 0;

        pthread_mutex_init(&list_list[i]->mutex, 0);
        list_list[i] -> spin_lock = 0;
        list_list[i] -> list = list_spot;
    }


    // Create all elements that are to be inserted into the list
    SortedListElement_t** all_elements = malloc(sizeof(SortedListElement_t*)*iteration_count*thread_count);
    if(!all_elements){
        fprintf(stderr, "Unable to allocate memory for the pointers to the elements of the linked list: %s\n", strerror(errno));
        FFlush(stderr);
        exit_status = 1;
    }
    int success = 0;
    for(int i = 0; i<iteration_count*thread_count; i++){
        all_elements[i] = malloc(sizeof(SortedListElement_t));
        // Error reporting
        if(!all_elements[i]){
           fprintf(stderr, "Unable to allocate memory for an element of the linked list: %s\n", strerror(errno));
            FFlush(stderr);
            exit_status = 1; 
        }
        all_elements[i]->key = generate_and_allocate_random_string(10, &success);
        exit_status = exit_status | success;
    }


    // Allocate space for threads
    pthread_t* threads = malloc(sizeof(pthread_t)*thread_count);
    if(!threads){
        fprintf(stderr, "Unable to allocate memory for threads: %s\n", strerror(errno));
        FFlush(stderr);
        exit_status = 1;
    }

    // Allocate arguments for each thread
    struct list_arguments** arg_pointer = malloc(sizeof(struct list_arguments*)*thread_count);
    if(!arg_pointer){
        fprintf(stderr, "Unable to allocate memory for the pointers to the elements of the linked list: %s\n", strerror(errno));
        FFlush(stderr);
        exit_status = 1;
    }
    for(int i = 0; i<thread_count; i++){
        arg_pointer[i] = malloc(sizeof(struct list_arguments));
        if(!arg_pointer[i]){
            fprintf(stderr, "Unable to allocate memory for arguments of a list: %s\n", strerror(errno));
            FFlush(stderr);
            exit_status = 1;
        }
        arg_pointer[i]->locks = sync_type;
        arg_pointer[i]->elements = all_elements+i*iteration_count;
        arg_pointer[i]->count = iteration_count;
        arg_pointer[i]->wait_time = 0;
        arg_pointer[i]->lists = list_list;
        arg_pointer[i]->list_count = list_count;
    }

    // Get clock starting time
    if(clock_gettime(CLOCK_REALTIME,&init_time)){
		fprintf(stderr, "Unable to get inital clock time: %s", strerror(errno));
        FFlush(stderr);
		exit_status = 1;
	}
    //DO COMPUTATION HERE
    for(int i = 0; i<thread_count; i++){
        struct list_arguments* value = arg_pointer[i];
		if(pthread_create(&(threads[i]), 0, threaded_function_run, (void*)value)){
            fprintf(stderr, "Unable to spawn more threads: %s\n", strerror(errno));
            FFlush(stderr);
            exit_status = 1;
        }
    }
    // Join threads
    for(int i = 0; i<thread_count; i++){
        if(pthread_join(threads[i], 0)){
			exit_status = 1;
			fprintf(stderr, "Unable to join a thread\n");
			FFlush(stderr);
		}
    }

    if(clock_gettime(CLOCK_REALTIME,&final_time)){
		fprintf(stderr, "Unable to get final clock time: %s\n", strerror(errno));
        FFlush(stderr);
		exit_status = 1;
	}

    for(int i = 0; i<list_count; i++){
        if(SortedList_length(list_list[i]->list)){
            fprintf(stderr, "List was not completely removed, indicating corrupted list\n");
            FFlush(stderr);
            exit_status = 2;
        }
    }
    long long wait_time = 0;
    // Sum up the wait times for each thread
    for(int i = 0; i<thread_count; i++){
        wait_time += arg_pointer[i]->wait_time;
    }

    // Print results here
    long long nsec = 0;
    nsec += 1e9*(final_time.tv_sec-init_time.tv_sec);
    nsec += final_time.tv_nsec - init_time.tv_nsec;
    long long op_count = thread_count * iteration_count * 3;
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", program_name, thread_count, iteration_count, list_count, op_count, nsec, nsec/op_count, wait_time/op_count);
    FFlush(stdout);

    // delete the whole array of items and free memory associated with keys
    for(int i =0; i<thread_count*iteration_count; i++){
        free((void*)(all_elements[i]->key));
        free(all_elements[i]);
    }
    for(int i = 0; i<thread_count; i++){
        free(arg_pointer[i]);
    }
    for(int i = 0; i<list_count; i++){
        free(list_list[i]->list);
        free(list_list[i]);
    }
    free(arg_pointer);
    free(list_list);
    free(all_elements);
    free(threads);

    return exit_status;
}


// Define the functions to be called by the threads
void add_to_list(locked_list** list, SortedListElement_t* element, int lock_flags, long long* wait_time, int list_count);
SortedListElement_t* lookup(locked_list** list, const char* key, int lock_flags, long long* wait_time, int list_count);
int delete(locked_list** list, SortedListElement_t* element, int lock_flags, long long* wait_time, int list_count);
int length(locked_list** list, int lock_flags, long long* wait_time, int list_count);
// the actual threaded function to insert, lookup and delete an element from a shared list
void* threaded_function_run(void* args){
    struct list_arguments* a = (struct list_arguments*) args;
    for(int i = 0; i<a->count; i++){
        add_to_list(a->lists, a->elements[i], a->locks, &a->wait_time, a->list_count);
    }
    if(length(a->lists, a->locks, &a->wait_time, a->list_count) == -1){
        fprintf(stderr, "Unable to get list length due to corrupted pointers, exiting\n");
        exit(2);
    }
    for(int i = 0; i<a->count; i++){
        SortedListElement_t* todelete = lookup(a->lists, a->elements[i]->key, a->locks, &a->wait_time, a->list_count);
        int failed_delete = 0;
        if(todelete)
            failed_delete += delete(a->lists, todelete, a->locks, &a->wait_time, a->list_count);
        else{
            fprintf(stderr, "Unable to locate element due to corrupted pointers, exiting\n");
            exit(2);
        }
        if(failed_delete){
            fprintf(stderr, "Unable to delete element due to corrupted pointers, exiting\n");
            exit(2);
        }
    }
    return 0;
}

// Add an element to a list
void add_to_list(locked_list** list, SortedListElement_t* element, int lock_flags, long long* wait_time, int list_count){
    unsigned long hash_val = hash(element->key);
    locked_list* select_list = list[hash_val%list_count];
    struct timespec begin, end;
    if(lock_flags & MUTEX_LOCK){
        clock_gettime(CLOCK_REALTIME,&begin);
        pthread_mutex_lock(&select_list->mutex);
        clock_gettime(CLOCK_REALTIME,&end);
        SortedList_insert(select_list->list, element);
        pthread_mutex_unlock(&select_list->mutex);
    }
    if(lock_flags & SPIN_LOCK){
        clock_gettime(CLOCK_REALTIME,&begin);
        while(__sync_lock_test_and_set(&select_list->spin_lock, 1))
            continue;
        clock_gettime(CLOCK_REALTIME,&end);
        SortedList_insert(select_list->list, element);
        __sync_lock_release(&select_list->spin_lock);
    }
    if(!lock_flags){
        SortedList_insert(select_list->list, element);
        return;
    }
    (*wait_time) += nsec_difference(&begin, &end);
}

// Get a reference to an element in a list
SortedListElement_t* lookup(locked_list** list, const char* key, int lock_flags, long long* wait_time, int list_count){
    unsigned long hash_val = hash(key);
    locked_list* select_list = list[hash_val%list_count];
    struct timespec begin, end;
    SortedListElement_t* element;
    if(lock_flags & MUTEX_LOCK){
        clock_gettime(CLOCK_REALTIME,&begin);
        pthread_mutex_lock(&select_list->mutex);
        clock_gettime(CLOCK_REALTIME,&end);
        element = SortedList_lookup(select_list->list, key);
        pthread_mutex_unlock(&select_list->mutex);
    }
    if(lock_flags & SPIN_LOCK){
        clock_gettime(CLOCK_REALTIME,&begin);
        while(__sync_lock_test_and_set(&select_list->spin_lock, 1))
            continue;
        clock_gettime(CLOCK_REALTIME,&end);
        element = SortedList_lookup(select_list->list, key);
        __sync_lock_release(&select_list->spin_lock);
    }
    if(!lock_flags)
        return SortedList_lookup(select_list->list, key);
    (*wait_time) += nsec_difference(&begin, &end);
    return element;
}

// Delete an element from a list
int delete(locked_list** list, SortedListElement_t* element, int lock_flags, long long* wait_time, int list_count){
    unsigned long hash_val = hash(element->key);
    locked_list* select_list = list[hash_val%list_count];
    struct timespec begin, end;
    int rval = 0;
    if(lock_flags & MUTEX_LOCK){
        clock_gettime(CLOCK_REALTIME,&begin);
        pthread_mutex_lock(&select_list->mutex);
        clock_gettime(CLOCK_REALTIME,&end);
        rval = SortedList_delete(element);
        pthread_mutex_unlock(&select_list->mutex);
    }
    if(lock_flags & SPIN_LOCK){
        clock_gettime(CLOCK_REALTIME,&begin);
        while(__sync_lock_test_and_set(&select_list->spin_lock, 1))
            continue;
        clock_gettime(CLOCK_REALTIME,&end);
        rval = SortedList_delete(element);
        __sync_lock_release(&select_list->spin_lock);
    }
    if(!lock_flags)
        return SortedList_delete(element);
    (*wait_time) += nsec_difference(&begin, &end);
    return rval;
}

// Get the length of a list
int length(locked_list** list, int lock_flags, long long* wait_time, int list_count){
    int len =0;
    for(int i = 0; i<list_count; i++){
        locked_list* select_list = list[i];
        struct timespec begin, end;
        if(lock_flags & MUTEX_LOCK){
            clock_gettime(CLOCK_REALTIME,&begin);
            pthread_mutex_lock(&select_list->mutex);
            clock_gettime(CLOCK_REALTIME,&end);
            len += SortedList_length(select_list->list);
            pthread_mutex_unlock(&select_list->mutex);
        }
        if(lock_flags & SPIN_LOCK){
            clock_gettime(CLOCK_REALTIME,&begin);
            while(__sync_lock_test_and_set(&select_list->spin_lock, 1))
                continue;
            clock_gettime(CLOCK_REALTIME,&end);
            len += SortedList_length(select_list->list);
            __sync_lock_release(&select_list->spin_lock);
        }
        if(!lock_flags)
            len += SortedList_length(select_list->list);
        else
            (*wait_time) += nsec_difference(&begin, &end);
    }
    return len;
}