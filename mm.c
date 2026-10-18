/*
 * mm.c
 *
 * Name: Ryan Cao
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 * Also, read the README carefully and in its entirety before beginning.
 *
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
void* get_header(void *ptr);
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
    char *padding = mm_sbrk(8);
    
    if (padding == NULL) {return false;}
    return true;
}

// helper function for setting header values
void* set_header(void *block, size_t size, uint64_t allocated_bit) {
  if (block == NULL) {return NULL;}
  
  uint64_t *header = (uint64_t*) block;
  
  *header = size;
  *header = *header + allocated_bit;
  
  return header;
}

// helper function for getting payload pointer
void *get_payload(void *block) {

    if (block == NULL) {return NULL;}
    
    void *payload = (void*) ((char*)block + 8);
    return payload;
}

// helper function for setting footer values
void *set_footer(void *block, size_t size, uint64_t allocated_bit) {

  if (block == NULL) {return NULL;}
  
  uint64_t *footer = (uint64_t*) ((char*)block + size - 8);
  
  *footer = size;
  *footer = *footer + allocated_bit;
  return footer;
}

void *split_block (void *payload, size_t size){

  if (payload == NULL) {return NULL;}
  
  uint64_t *header = (uint64_t*) get_header(payload);
  
  size_t requested = size + 16;
  size_t block_size = *header;
  size_t remaining = block_size - requested;
  
  // remaining block must be at least 32 bytes to split
  if (remaining >= 32){
  
    void *block = (char*)payload + requested - 8; // pointer to the header of remaining block
    
    
    void *header = set_header(block, remaining, 0);
    void *payload = get_payload(block);
    void *footer = set_footer(block, remaining, 0);
    
    if ((header == NULL) | (payload == NULL) | (footer == NULL)) {return NULL;}
    
    return payload;
  }
  
  return NULL;
}
void *find_free_block(size_t size) {

  void *curr = head;
  
  while (curr != 0) {
  
    // get the size of the current block
    void *header = get_header(curr);
    
    size_t requested = size + 16;
    size_t block_size = *(uint64_t*)header;
    
    // compare the size with the requested size
    if (block_size >= requested) {
    
      // only split if left over block is at least 32 bytes
      return curr;
    }
    
    // move to next block
    curr = (void*) (((uint64_t*)curr) + 1);
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
    
    //printf("Aligned size: %ld\n", aligned_size);
    void *new_block = find_free_block(aligned_size);
    
    
    if (new_block == NULL) {
    
      // failed to find a free block - request more space
      new_block = mm_sbrk(aligned_size + 16);
    } else {
    
      list_remove(new_block);
      void *leftover = split_block(new_block, aligned_size); // payload of remaining block (if any)
      
      if (leftover != NULL) {
        list_insert(leftover);
      }
    }
    
    if (new_block == NULL) {return NULL;}
    
    //printf("mm_sbrk success\n"); // REMOVE LATER
    
    // segment the new block into header, payload, and footer
    uint64_t *header = (uint64_t*) set_header(new_block, aligned_size, 1); // creates a pointer for the header, and sets header's values
    uint64_t *payload = (uint64_t *) get_payload(new_block); // creates a pointer for the payload
    uint64_t *footer = (uint64_t *) set_footer(new_block, aligned_size, 1); // creates a pointer for the footer, and sets footer's values
    
    if ((footer == NULL) | (header == NULL) | (payload == NULL)) {return NULL;}
    
    //printf("header address: %p  payload address: %p  footer address: %p\n", header, payload, footer); // REMOVE LATER
    
    return payload;
}

/*
 * list_insert
 */
void list_insert(void *payload){

  // always insert at the head
  
  if (payload == NULL) {return;}
  
  // store prev and next inside the payload
  // prev pointer is first 8 bytes, next pointer is next 8 bytes
  uint64_t *block = (uint64_t*) payload;
  
  // insert into empty list
  if (head == 0) {
    head = block;
    block[0] = 0;
    block[1] = 0;
  } else {
  // insert at head
    block[0] = 0;
    block[1] = (uint64_t) head;
    head = block;
  }
}

// removes a block from the list
void list_remove(void *ptr){
  if (ptr == NULL) {return;}

  uint64_t *curr = (uint64_t*) ptr;
  
  if (head == ptr){
    head = 0;
  }
  
  // link previous block with next block
  if (curr[0] != 0) {
    uint64_t *prev = (uint64_t*) curr[0];
    prev[1] = curr[1];
  }
  
  // link next block with previous block
  if (curr[1] != 0) {
    uint64_t *next = (uint64_t*) curr[1];
    next[0] = curr[0];
  }
  
  // clear current block
  curr[0] = 0;
  curr[1] = 0;
}


void* get_header(void *ptr) {
  if (ptr == NULL) {return NULL;}
  void *header = (void*) ((char*)ptr - 8);
  
  return header;
}

void* get_footer (void *ptr, size_t payload_size) {
  if (ptr == NULL) {return NULL;}
  void *footer = (void*) ((char*) ptr + payload_size);
  return footer;
}


/*
 * free
 */
void free(void* ptr)
{
    if (ptr == NULL) {return;}
    
    // clear the allocated bit in header and footer
    
    uint64_t *header = (uint64_t*)get_header(ptr);
    uint64_t block_size = *header - 1;
    
    uint64_t payload_size = block_size - 16;
    
    uint64_t *footer = (uint64_t*) get_footer(ptr, payload_size);
    
    // clear header bit
    *header = *header - 1;
    
    // clear footer bit;
    *footer = *footer - 1;
    return;
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
