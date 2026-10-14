#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>


// insert at tail
void list_insert(list_t *list, void *data){
  if (data == NULL) {return false;}
  
  list_node_t *node;
  
  node->prev = NULL;
  node->next = NULL;
  node->data = data;
  
  if (list->tail == NULL){
    list->head = node;
    list->tail = node;
    return;
  }
  
  list_node_t *tail_node = list->tail;
  
  tail_node->next = node;
  node->prev = tail_node;
  list->tail = node;
  
  return;
}

void list_remove(list_t *list, list_node_t *node){
  if (list == NULL || node == NULL) {return;}
  
  if (list->head == node) {
    list->head->next = NULL;
    
  }
}