#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>


void list_insert(list_t *list, void *block_p){

  // always insert at the tail
  
  
  if (data == NULL) {return false;}
  
  // store field data inside the free block itself
  list_node_t *node = (list_node_t *) block_p;
  node->prev = NULL;
  node->next = NULL;
  
  // insert in empty list
  if (list->head == NULL) {
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

void list_remove(list_t *list, list_node_t *node){
  if (list == NULL || node == NULL) {return;}
  
  if (list->head == node) {
    list->head->next = NULL;
    
  }
}