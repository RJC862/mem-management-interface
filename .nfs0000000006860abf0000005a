/*
 * mm.c
 *
 * Name: Ryan Cao
 *
 * custom helper functions:
 *   get_header(), get_footer(), get_payload(),
 *   set_header(), set_footer(),
 *   find_free_block(), split_block()
 *   list_insert(), list_remove()
 * 
 * High Level Implementation Description:
 *  
 *  The project implements malloc() and free(), as well as realloc() and calloc().
 *   
 *  A doubly linked list is used to track free blocks, and splitting and coalescing are utilized to minimize wasted memory.
 *
 *  malloc() - allocates n bytes of memory requested by the caller.
 *  The mechanism behind malloc() works as such: 
 *    1. A free list is searched for a free block of suitable size. In this case, the suitable size is at least n + 16 bytes, because we also require space for the header and footer.
 *    2. If a free block of suitable size is found, but its size is at least 32 bytes larger than the smallest suitable size, we split it and return the remaining chunk back to the     *       free list. Otherwise do nothing.
 *    3. The free block is removed from the list, its header and footer are set appropriately, and the payload is given to the caller.
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"
#include "linked_list.h"

/*
 * If you want to enable your debugging output and heap checker code,
 * uncomment the following line. Be sure not to have debugging enabled
 * in your final submission.
 */
// #define DEBUG

#ifdef DEBUG
// When debugging is enabled, the underlying functions get called
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#else
// When debugging is disabled, no code gets generated
#define dbg_printf(...)
#define dbg_assert(...)
#endif // DEBUG

// do not change the following!
#ifdef DRIVER
// create aliases for driver tests
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mm_memset
#define memcpy mm_memcpy
#endif // DRIVER

#define ALIGNMENT 16





void *get_header(void *ptr);
void list_remove(void *ptr);


// rounds up to the nearest multiple of ALIGNMENT
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}

/*
 * mm_init: returns false on error, true on success.
 */
bool mm_init(void)
{
    head = 0; // reset free list
    
    uint64_t *padding = mm_sbrk(8); 
    
    if (padding == NULL) {return false;}
    
    
    // padding is always allocated; easy checking for start of heap
    *padding = 1;
    
    return true;
}

// helper function for setting header values
void set_header(void *header, size_t size, uint64_t allocated_bit) {
  if (header == NULL) {return;}
  uint64_t *new_header = (uint64_t *) header;
  
  *new_header = size;
  *new_header = *(uint64_t*)header + allocated_bit;
}

// helper function for getting payload pointer
void *get_payload(void *header) {

    if (header == NULL) {return NULL;}
    
    void *payload = (void*) ((char*)header + 8);
    return payload;
}

// helper function for setting footer values
void set_footer(void *header, size_t size, uint64_t allocated_bit) {

  if (header == NULL) {return;}
  
  uint64_t *footer = (uint64_t*) ((char*)header + size - 8);
  
  *footer = size;
  *footer = *footer + allocated_bit;
}

// splits the block header points to
// returns a pointer to the header of the remaining block
void *split_block (void *header, size_t size){

  if (header == NULL) {return NULL;}
  
  uint64_t allocated = *(uint64_t*)header & 1;
  size_t block_size = *(uint64_t*)header - allocated;
  size_t remaining = block_size - size;
  
  // remaining block must be at least 32 bytes to split
  if (remaining >= 32){
    
    void *r_block = (char*)header + size; // pointer to the header of remaining block
    
    // set header and footer values
    set_header(r_block, remaining, 0);
    set_footer(r_block, remaining, 0);
    
    return r_block;
  }
  
  return NULL;
}

// finds a suitable free block in the free list
// returns a pointer to the head of the first free block it finds
void *find_free_block(size_t size) {

  void *curr = head;
  
  while (curr != 0) {
  
    // get the size of the current block
    uint64_t allocated = *(uint64_t *)curr & 1;
    size_t block_size = *(uint64_t*)curr - allocated;
    
    // compare the size with the requested size
    if (block_size >= size) {
    
      return curr;
    }
    
    // move to next block
    
    uint64_t *payload = (uint64_t *)get_payload(curr);
    curr = (void *) payload[1];
  }
  
  // failed to find a suitable block
  return NULL;
}
/*
 * malloc
 */
void* malloc(size_t size)
{
    //printf("malloc'd: %ld Bytes\n", size); // REMOVE LATER
    
    size_t aligned_size = align(size);
    
    size_t required_size = aligned_size + 16; // 16 bytes for header and footer
    
    //printf("Aligned size: %ld\n", aligned_size);
    
    size_t block_size = required_size;
    
    void *new_block = find_free_block(required_size);
    
    if (new_block == NULL) {
    
      // failed to find a free block - request more space
      
      new_block = mm_sbrk(required_size);
      
      if (new_block == NULL) {return NULL;}
      
      set_header(new_block, block_size, 1);
      set_footer(new_block, block_size, 1);
      
      return get_payload(new_block);
      
    } else {
      // split the free block if necessary
      
      list_remove(new_block);
      
            
      uint64_t allocated = *(uint64_t *) new_block & 1;
      block_size = *(uint64_t *) new_block - allocated;
      
      void *leftover_header = split_block(new_block, required_size);
      
      if (leftover_header != NULL) {
        block_size = required_size;
        list_insert(leftover_header);
      }
      
      set_header(new_block, block_size, 1);
      set_footer(new_block, block_size, 1);
      
      return get_payload(new_block);
    }
}

// insert a free block at the head of the free list
// returns nothing
void list_insert(void *header){
  if (header == NULL) {return;}
  
  // store prev and next inside the payload
  // prev pointer is first 8 bytes, next pointer is next 8 bytes
  uint64_t *payload = (uint64_t*) get_payload(header);
  
  payload[0] = 0;
  payload[1] = (uint64_t) head;
  
  // set the head's prev data to point to the new block's header
  if (head != 0){
    uint64_t *head_payload = (uint64_t *) get_payload(head);
    
    head_payload[0] = (uint64_t) header;
  }
  
  head = header;
}

// removes the block at header from the free list
// returns nothing
// payload[0] points to the previous block's header, payload[1] the next block's header
void list_remove(void *header){
  if (header == NULL) {return;}

  uint64_t *payload = (uint64_t*) get_payload(header);
  
  assert(((uintptr_t) header & 0xF) == 8);                       // headers should end in 8
  assert(!(*(uint64_t *) header & 1));                           // must be a free block


  void *prev = (void *) payload[0];
  void *next = (void *) payload[1];
  
  // link previous block to the next block
  if (prev != 0) {
    uint64_t *prev_payload = (uint64_t *) get_payload(prev);
    prev_payload[1] = (uint64_t) next;
  } else {
    head = next; // the removed block was the head
  }
  
  // link the next block to the previous block
  if (next != 0) {
    uint64_t *next_payload = (uint64_t *) get_payload(next);
    next_payload[0] = (uint64_t) prev;
  }
  
  // clear prev and next data
  payload[0] = 0;
  payload[1] = 0;
}

// merges free blocks neighboring header's block
// returns the header of the merged block
void *merge(void *header, size_t block_size){

  if (header == NULL) {return NULL;}
  
  // left neighbor footer
  uint64_t *left_neighbor = (uint64_t *) header - 1;
  
  // right neighbor header
  uint64_t *right_neighbor = (uint64_t*) ((char*) header + block_size);
  
  void *end_of_heap = mm_heap_hi();
  
  // start of heap has a sentinel block, the padding - don't need to worry about left neighbor
  // end of heap does not have sentinel, so we need to check addresses - worry about right neighbor
  uint64_t left_allocated = *left_neighbor & 1;
  uint64_t right_allocated;
  
  // end of heap bounds checking
  if ((char *) right_neighbor + 8 >= (char *) end_of_heap + 1) {
    right_allocated = 1;
  } else {
    right_allocated = *right_neighbor & 1;
  }
  
  if (left_allocated && right_allocated) { // both neighbors allocated
  
    return header;
    
  } else if (right_allocated == 0 && left_allocated) { // right neighbor free, left neighbor allocated
  
    list_remove(right_neighbor);
    
    size_t r_size = *right_neighbor;
    
    // merge current block and right neighbor
    size_t merge_size = block_size + r_size;
    
    set_header(header, merge_size, 0);
    set_footer(header, merge_size, 0);
    
    // put merged block in the free list
    return header;
    
  } else if (left_allocated == 0 && right_allocated) { // left neighbor free, right neighbor allocated
  
    size_t l_size = *left_neighbor;
    
    // use the left neighbor's header for consistency in conventions
    uint64_t *l_header = (uint64_t *) ((char *) header - l_size);
    
    list_remove(l_header);

    // merge current block and left neighbor
    size_t merge_size = block_size + l_size;
    
    set_header(l_header, merge_size, 0);
    set_footer(l_header, merge_size, 0);
    
    // put merged block in the free list
    return l_header;
    
  } else { // both neighbors free
  
    list_remove(right_neighbor);
    
    size_t l_size = *left_neighbor;
    
    size_t r_size = *right_neighbor;
    
    // use the left neighbor's header for consistency in conventions
    uint64_t *l_header = (uint64_t *) ((char *) header - l_size);
    
    list_remove(l_header);
    
    size_t merge_size = block_size + l_size + r_size;
    set_header(l_header, merge_size, 0);
    set_footer(l_header, merge_size, 0);
  
    return l_header;
  }

}

void *get_header(void *payload) {
  if (payload == NULL) {return NULL;}
  
  void *header = (void *) ((char *) payload - 8);
  
  return header;
}
void *get_footer(void *header){

  if (header == NULL) {return NULL;}
  
  uint64_t allocated = *(uint64_t*)header & 1;
  uint64_t block_size = *(uint64_t*)header - allocated;
  
  void *footer = (void *) ((char *)header + block_size - 8);
  
  return footer;
}

/*
 * free
 */
void free(void* payload)
{
    if (payload == NULL) {return;}
    
    // free the block
    
    uint64_t *header = (uint64_t *) get_header(payload);
    
    
    uint64_t allocated = *(uint64_t *) header & 1;
    size_t block_size = *header - allocated;
    
    uint64_t *footer = (uint64_t*) get_footer(header);

    // clear allocated bit
    *header = *header - allocated;
    *footer = *footer - allocated;
    
    
    // merge any free neighbors
    void *merged = merge(header, block_size);
    
    if (merged == NULL) {return;}
    list_insert(merged);
}
/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{
    if (oldptr == NULL) {
      void *new_block = malloc(size);
      
      if (new_block == NULL) {return NULL;}
      
      new_block = memcpy(new_block, oldptr, size);
      
      free(oldptr);
      oldptr = NULL;
      
      return new_block;
    }
    
    if (size == 0) {
      free(oldptr);
      oldptr = NULL;
      
      return NULL;
    }
    
    
    void *new_block = malloc(size);
    new_block = memcpy(new_block, oldptr, size);
    
    if (new_block == NULL) {
      free(new_block);
      return NULL;
    }
    
    free(oldptr);
    oldptr = NULL;
    
    return new_block;
}

/*
 * calloc
 * This function is not tested by mdriver, and has been implemented for you.
 */
void* calloc(size_t nmemb, size_t size)
{
    void* ptr;
    size *= nmemb;
    ptr = malloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/*
 * Returns whether the pointer is in the heap.
 * May be useful for debugging.
 */
static bool in_heap(const void* p)
{
    return p <= mm_heap_hi() && p >= mm_heap_lo();
}

/*
 * Returns whether the pointer is aligned.
 * May be useful for debugging.
 */
static bool aligned(const void* p)
{
    size_t ip = (size_t) p;
    return align(ip) == ip;
}

/*
 * mm_checkheap
 * You call the function via mm_checkheap(__LINE__)
 * The line number can be used to print the line number of the calling
 * function where there was an invalid heap.
 */
bool mm_checkheap(int line_number)
{
#ifdef DEBUG
    // Write code to check heap invariants here
    // IMPLEMENT THIS
#endif // DEBUG
    return true;
}
