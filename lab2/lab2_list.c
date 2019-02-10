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
#include <signal.h>
#include <unistd.h>

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
    SortedList_t* list;
    // yielding is done with an extern variable in SortedList.c
    int count;
    SortedListElement_t** elements;
};
// assistant function to print out a list
void printList(SortedList_t* list){
    SortedListElement_t* current = list->next;
    while(current){
        fprintf(stdout, "%s, ", current->key);
        current = current -> next;
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

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
SortedListElement_t* lookup(SortedList_t* list, const char* key, int lock_flags){
    SortedListElement_t* element;
    if(lock_flags & MUTEX_LOCK){
        pthread_mutex_lock(&mutex);
        element = SortedList_lookup(list, key);
        pthread_mutex_unlock(&mutex);
        return element;
    }
    if(lock_flags & SWAP_LOCK){
        //TODO
    }
    return SortedList_lookup(list, key);
}
int delete(SortedListElement_t* element, int lock_flags){
    int rval = 0;
    if(lock_flags & MUTEX_LOCK){
        pthread_mutex_lock(&mutex);
        rval = SortedList_delete(element);
        pthread_mutex_unlock(&mutex);
        return rval;
    }
    if(lock_flags & SWAP_LOCK){
        //TODO
    }
    return SortedList_delete(element);
}

// the actual threaded function
void* threaded_function_run(void* args){
    struct list_arguments* a = (struct list_arguments*) args;
    for(int i = 0; i<a->count; i++){
        add_to_list(a->list, a->elements[i], a->locks);
    }
    if(SortedList_length(a->list) == -1){
        fprintf(stderr, "Unable to get list length due to corrupted pointers, exiting\n");
        exit(2);
    }
    for(int i = 0; i<a->count; i++){
        SortedListElement_t* todelete = lookup(a->list, a->elements[i]->key, a->locks);
        int failed_delete = 0;
        if(todelete)
            failed_delete += delete(todelete, a->locks);
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

void signal_handle(int sig){
    write(STDERR_FILENO, "Segmentation Fault, likely due to a corrupted list. Exiting\n", 61);
    (void)sig; // suppress compiler warning about variable
    _exit(2);
}

int opt_yield = 0;

int main(int argc, char** argv){
    srand(time(0)); // set (pseudo)random seed based on time
    int exit_status = 0;
    // set up signal handler
    if(SIG_ERR == signal(SIGSEGV, signal_handle)){ 
        fprintf(stderr, "Error setting signal handler: %s", strerror(errno));
        exit_status = 1;
    }
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
                        sync_type = SWAP_LOCK;
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
    // set up string for print out value
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
        if(sync_type & SWAP_LOCK){
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
    SortedList_t* list = malloc(sizeof(SortedList_t));
    if(!list){
        fprintf(stderr, "Unable to allocate memory for the head of the linked list: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        exit_status = 1;
    }
    list -> key = 0;
    list -> next = 0;
    list -> prev = 0;

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
        // Error reporting
        if(!all_elements[i]){
           fprintf(stderr, "Unable to allocate memory for an element of the linked list: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        exit_status = 1; 
        }
        all_elements[i]->key = generate_and_allocate_random_string(10, &success);
        exit_status = exit_status | success;
    }


    // Allocate space for threads
    pthread_t* threads = malloc(sizeof(pthread_t)*thread_count);
    if(!threads){
        fprintf(stderr, "Unable to allocate memory for threads: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        exit_status = 1;
    }

    // Allocate arguments for each thread
    struct list_arguments** arg_pointer = malloc(sizeof(struct list_arguments*)*thread_count);
    if(!arg_pointer){
        fprintf(stderr, "Unable to allocate memory for the pointers to the elements of the linked list: %s\n", strerror(errno));
        if(fflush(stderr)){
            fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
        }
        exit_status = 1;
    }
    for(int i = 0; i<thread_count; i++){
        arg_pointer[i] = malloc(sizeof(struct list_arguments));
        if(!arg_pointer[i]){
            fprintf(stderr, "Unable to allocate memory for arguments of a list: %s\n", strerror(errno));
            if(fflush(stderr)){
                fprintf(stderr, "Unable to flush stderr: %s\n", strerror(errno));
            }
            exit_status = 1;
        }
        arg_pointer[i]->locks = sync_type;
        arg_pointer[i]->elements = all_elements+i*iteration_count;
        arg_pointer[i]->count = iteration_count;
        arg_pointer[i]->list = list;
    }

    // Get clock starting time
    if(clock_gettime(CLOCK_REALTIME,&init_time)){
		fprintf(stderr, "Unable to get inital clock time: %s", strerror(errno));
		exit_status = 1;
	}
    //DO COMPUTATION HERE
    for(int i = 0; i<thread_count; i++){
        struct list_arguments* value = arg_pointer[i];
		if(pthread_create(&(threads[i]), 0, threaded_function_run, (void*)value)){
            fprintf(stderr, "Unable to spawn more strings: %s\n", strerror(errno));
            if(fflush(stderr)){
                fprintf(stderr, "Unable to flush stderr buffer: %s\n", strerror(errno));
            }
            exit_status = 1;
        }
    }
    // Join threads
    for(int i = 0; i<thread_count; i++){
        if(pthread_join(threads[i], 0)){
			exit_status = 1;
			fprintf(stderr, "Unable to join a thread\n");
			if(fflush(stderr))
                fprintf(stderr, "Unable to flush stderr buffer: %s\n", strerror(errno));
		}
    }

    if(clock_gettime(CLOCK_REALTIME,&final_time)){
		fprintf(stderr, "Unable to get final clock time: %s", strerror(errno));
		exit_status = 1;
	}

    if(SortedList_length(list))
        exit_status = 2;

    // Print results here
    long long nsec = 0;
    nsec += 1e9*(final_time.tv_sec-init_time.tv_sec);
    nsec += final_time.tv_nsec - init_time.tv_nsec;
    long long op_count = thread_count * iteration_count * 3;
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld\n", program_name, thread_count, iteration_count, 1, op_count, nsec, nsec/op_count);
    fflush(stdout);

    // delete the whole array of items and free memory associated with keys
    for(int i =0; i<thread_count*iteration_count; i++){
        free((void*)(all_elements[i]->key));
        free(all_elements[i]);
    }
    for(int i = 0; i<thread_count; i++){
        free(arg_pointer[i]);
    }
    free(arg_pointer);
    free(list);
    free(all_elements);
    free(threads);

    return exit_status;
}