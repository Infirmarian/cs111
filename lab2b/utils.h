//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
typedef struct SortedListElement SortedList_t;
void FFlush(FILE* stream);
char* generate_and_allocate_random_string(int length, int* success);
unsigned long hash(const char* str);
long long nsec_difference(struct timespec* begin, struct timespec* end);
void printList(SortedList_t* list);
#endif