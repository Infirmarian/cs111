//NAME: Robert Geil
//EMAIL: rgeil@ucla.edu
//ID: 104916969

#include "SortedList.h"
#include <string.h>
/**
 * SortedList_insert ... insert an element into a sorted list
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    if(list->next == 0){
        list->next = element;
        element->prev = list;
    }
    while(list->next){
        if(list->next->key < element->key){
            list = list->next;
        }else{
            SortedList_t* temp = list->next;
            list->next = element;
            element->prev = list;
            element->next = temp;
            temp->prev = element;
            return;
        }
    }
    list->next = element;
    element->prev = list;

}
/**
 * SortedList_delete ... remove an element from a sorted list
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 * @param SortedListElement_t *element ... element to be removed
 * @return 0: element deleted successfully, 1: CORRTUPED prev/next pointers
 */
int SortedList_delete(SortedListElement_t *element){
    // check to make sure that the element exists correctly. Exception for the ending node
    if((element->next && element->next->prev != element) || element->prev->next != element){
        return 1;
    }
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}
/**
 * SortedList_lookup ... search sorted list for a key
 *	The specified list will be searched for an
 *	element with the specified key.
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    while(list->next){
        if(!strcmp(list->next->key, key)){
            return list->next;
        }
        list = list->next;
    }
    return 0;
}
/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 * @param SortedList_t *list ... header for the list
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list){
    int count = 0;
    while(list->next){
        if(list->next->prev != list)
            return -1;
        count ++;
        list = list ->next;
    }
    return count;
}