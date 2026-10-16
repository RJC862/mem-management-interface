#include <stdio.h>
#include <stdlib.h>


typedef struct list_node{
  struct list_node prev;
  struct list_node next;
  void *data;
} list_node_t;


typedef struct list{
  list_node_t *head;
  list_node_t *tail;
  size_t *count;
}list_t;

void *list_create();

bool list_insert(void *data)

bool list_remove(void *data)