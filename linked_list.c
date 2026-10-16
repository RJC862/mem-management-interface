#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>

void list_insert(void *head, void *payload){

  // always insert at the head
  
  if (data == NULL) {return false;}
  
  // store field data inside the payload
  uint64_t *block = (uint64_t*) payload
  
  // insert into empty list
  if (head == NULL) {
    head = payload;
  }
  
  
  
  
  // insert in empty list
  if (list->count == 0) {
    list->head = node;
    list->tail = node;
    list->count++;
    return;
  }
  
  // insert at tail
  list->tail->next = node;
  node->prev = list->tail;
  list->tail = node;
  
  
  return;
}


// removes a node from the list
void list_remove(list_t *list, list_node_t *node){
  if (list == NULL || node == NULL) {return;}
  
  if (list->head == node && list->tail == node) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
  } else if (list->head == node) {
    list_node_t next = node->next;
    
    node->next = NULL;
    list->head = temp;
    list->count--;
  } else if (list->tail == node){
    list_node_t prev = list->prev;
    
    node->prev = NULL;
  }
  
  // set allocate bit in header and footer
    char *mblock = (char *) node;
}