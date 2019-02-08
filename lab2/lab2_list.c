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

typedef int bool;
#define false 0
#define true 1

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

int opt_yield = 0;

int main(int argc, char** argv){
    srand(time(0)); // set (pseudo)random seed based on time
    int exit_status = 0;
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
    //TODO: generate strings
    char* randstring;
    int success = 0;
    SortedListElement_t* new_element;
    for(int i = 0; i<iteration_count; i++){
        randstring = generate_and_allocate_random_string(10, &success);
        exit_status = exit_status | success;
        new_element = malloc(sizeof(SortedListElement_t));
        new_element->key = randstring;
        SortedList_insert(head, new_element);
    }
    // print out
    SortedListElement_t* current = head->next;
    while(current){
        fprintf(stdout, "%s, ", current->key);
        current = current -> next;
    }
    fprintf(stdout, "\n");
    fflush(stdout);



    // delete the whole list and free memory
    SortedListElement_t* ptr = head->next;
    SortedListElement_t* prev;
    while(ptr){
        free((void*)(ptr->key));
        prev = ptr;
        ptr = ptr->next;
        free(prev);
    }
    free(head);

    return exit_status;
}