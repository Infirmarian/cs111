//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "SortedList.h"


// Creates a random string of given length
char* generate_and_allocate_random_string(int length, int* success){
    static char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
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

// Returns a good hash for any given string, using djb2 algorithm
// Source: http://www.cse.yorku.ca/~oz/hash.html 
unsigned long hash(const char* str){
    unsigned long hash_value = 5381;
        int c;
        while ((c = *str++))
            hash_value = ((hash_value << 5) + hash_value) + c;
        return hash_value;
}

// Give the nanosecond difference between two timespecs
// Potential for overflow, but dealing with short timespans (less than ~292 years) should be fine
long long nsec_difference(struct timespec* begin, struct timespec* end){
    long long diff = end->tv_sec - begin->tv_sec;
    diff *= 1e9;
    diff += end->tv_nsec - begin->tv_nsec;
    return diff;
}

// assistant function to print out a list. This isn't really called except for debuggin purposes
void printList(SortedList_t* list){
    SortedListElement_t* current = list->next;
    while(current){
        fprintf(stdout, "%s, ", current->key);
        current = current -> next;
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

void FFlush(FILE* stream){
    if(fflush(stream)){
        fprintf(stderr, "Unable to flush output: %s\n", strerror(errno));
    }
}