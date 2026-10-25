/*
 * mm.c
 *
 * Name: Ryan Cao
 *
 * custom helper functions:
 *   get_header(), get_payload(),
 *   set_header(), set_footer(),
 *   find_free_block(), split_block()
 *   list_insert(), list_remove(), size_to_list()
 *   merge()
 * 
 * High Level Implementation Description:
 *  
 *  The project implements malloc() and free(), as well as realloc() and calloc().
 *   
 *  A segregated list is used to track free blocks, and splitting and coalescing are utilized to minimize wasted memory. Footer optimization is also implemented so that allocated blocks do not have footers, and free blocks do, only for coalescing.
 *
 *  
 *
 *
 *  void *malloc(size_t size) - allocates size bytes of memory requested by the caller.
 *  The mechanism behind malloc() works as such: 
 *    1. A free list is searched for a free block of suitable size. In this case, the suitable size is at least size + 8 bytes, because we also require space for the header.
 *    2. If a free block of suitable size is found, but its size is at least 32 bytes larger than the smallest suitable size, we split it and return the remaining chunk back to the free list. Otherwise do nothing.
 *    3. The free block is removed from the list, its header is set appropriately, and the payload is returned to the caller.
 *    4. You might notice that padding is initialized to 16 bytes. This is because the first 8 bytes are used as padding, and the next 8 bytes initialize the allocated bits for the next block that's allocated. This is so that we don't have to check for start of heap bounds. The *       bits are built in so that when we coalesce, we know that the padding is allocated.
 *  
 *  void free(void *payload) - deallocates the block of memory containing payload.
 *  The mechanism behind free() works as such:
 *    1. First, the allocated bit is set to 0 for the header of the block containing payload. This effectively deallocates the memory block.
 *    2. Coalesce using merge(). Coalescing amends external fragmentation in the heap.
 *    3. Coalesced/Merged block is then added back to the free list.
 *
 *  void* realloc(void* oldptr, size_t size) - resizes the memory chunk at oldptr to size bytes
 *  The mechanism behind realloc() works as such:
 *    1. Call malloc(), for a chunk of memory of size bytes.
 *    2. Copy the bytes at oldptr to the new malloc'd block
 *    3. Return the new malloc'd payload
 *
 *  Note: The convention is usually to use header pointers as a base for pointer arithmetic.
 *        This is because free blocks point to each other's headers in the segregated list, 
 *        and the segregated list is the basis of memory management.
 *
 *  Custom helper functions are documented with in-function comments. 
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
//#define DEBUG

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
    // reset segregated list
    for (int i = 0; i < 16; i++) {
      heads[i] = 0;
    }
    
    uint64_t *padding = mm_sbrk(16); // 8 bytes for padding, 8 bytes to initialize a header for the first allocated block
    
    if (padding == NULL) return false;

    padding[0] = 0;
    padding[1] = 3;
    return true;
}


// helper function for setting header values
// takes as input the header pointer of a block, the size of the block, and the allocation bit for the block
// returns nothing
void set_header(void *header, size_t size, uint64_t allocated) {
  if (header == NULL) {return;}
  uint64_t *new_header = (uint64_t *) header;
  
  *new_header = size + (*new_header & 2) + allocated; // preserve bit 1
}

// helper function for getting payload pointer
// takes as input the header pointer of a block
// returns the payload pointer
void *get_payload(void *header) {

    if (header == NULL) {return NULL;}
    
    void *payload = (void*) ((char*)header + 8);
    return payload;
}

// helper function for setting footer values; use in tandem with set_header always
// takes as input the header pointer of a block, the size of the block, and the allocation bit for the block.
// returns nothing
void set_footer(void *header, size_t size, uint64_t allocated) {

  if (header == NULL) {return;}
  
  uint64_t *footer = (uint64_t*) ((char*)header + size - 8);
  
  *footer = size;
  *footer += allocated;
}

// splits the block header points to; called only in response to a malloc()
// takes as input the header pointer of a block and the required size to fulfill the malloc() call
// returns a pointer to the header of the remaining block
void *split_block (void *header, size_t size){

  if (header == NULL) {return NULL;}
  
  size_t block_size = *(uint64_t*)header & ~3;
  size_t remaining = block_size - size;
  
  // remaining block must be at least 32 bytes to split
  if (remaining >= 32){
    
    void *r_block = (char*)header + size; // pointer to the header of remaining block
    
    // set header and footer values
    set_header(r_block, remaining, 0);
    *(uint64_t *) r_block |= 2;
    
    set_footer(r_block, remaining, 0);
    
    return r_block;
  }
  
  return NULL;
}

// finds a suitable free block in the free list; called only in response to malloc()
// takes as input the required size to fulfill the malloc() call
// returns a pointer to the head of the first free block it finds
void *find_free_block(uint64_t list_index, size_t size) {

  assert(list_index <= 15);
  
  // search all lists starting from list_index
  while (list_index < 16) {
  
    void *curr = (void *) heads[list_index];
    
    
    while (curr != 0) {
      size_t block_size = *(uint64_t*)curr & ~3;
      
      // compare the size with the requested size
      if (block_size >= size) {
      
        return curr;
      
      }
      
      // move to next block
      
      uint64_t *payload = (uint64_t *)get_payload(curr);
      curr = (void *) payload[1];
    }
    list_index++;
  }
  
  // failed to find a suitable block
  return NULL;
}

// Takes as input the size of a block and returns the segregated list it should go in
uint64_t size_to_list(size_t size) {

  int index = -5;
  
  while (size > 1) {
    index++;
    size /= 2;
  }
  
  if (index >= 16){
    return 15; // blocka too large for the other lists go into the last list
  }
  return index;
}


// Updates bit 1 of the header of a block's right neighbor for footer optimization
// Takes as input the pointer to the current block, the size of the current block, and what to set bit 1 to for the right neighbor
void update_right_header(void *header, size_t size, uint64_t allocated){
  if (header == NULL) return;
  
  char *right_header = (char *) header + size;
  
  if (right_header == NULL) return;
  
  
  if (allocated) *right_header |= 2; // set bit 1 to 1
  else *right_header = *right_header & ~2; // clear bit 1
}

/*
 * malloc
 */
void* malloc(size_t size)
{

    // 8 bytes for header
    size_t required_size = align(size + 8);
    
    if (required_size < 32) {
      required_size = 32;
    }
    
    void *new_block = find_free_block(size_to_list(required_size), required_size);
    
    if (new_block == NULL) {
    
      // failed to find a free block - request more space
      
      char *temp = mm_sbrk(required_size);
      
      if (temp == NULL) return NULL;
      
      new_block = temp - 8;
      set_header(new_block, required_size, 1);
      *(uint64_t *) ((char *) new_block + required_size) = 3;
      
      return get_payload(new_block);
      
    }
    
    // split the free block if necessary
    
    list_remove(new_block);
    
    size_t block_size = *(uint64_t *) new_block & ~3;
    
    void *leftover_header = split_block(new_block, required_size);
    
    if (leftover_header != NULL) {
      block_size = required_size;
      
      list_insert(leftover_header);
    }
    
    set_header(new_block, block_size, 1);
    update_right_header(new_block, block_size, 1);
    
    return get_payload(new_block);
    
}


// inserts a free block at the head of the free list
// takes as input the header pointer of a block
// returns nothing
void list_insert(void *header){
  if (header == NULL) {return;}
  
  // calculate segregated list to insert into
  size_t size = *(uint64_t *) header & ~3;
  uint64_t idx = size_to_list(size);
  
  // store prev and next inside the payload
  // prev pointer is first 8 bytes, next pointer is next 8 bytes
  uint64_t *payload = (uint64_t*) get_payload(header);
  
  void *head = (void *) heads[idx];
  
  payload[0] = 0;
  payload[1] = (uint64_t) head;
  
  // set the head's prev data to point to the new block's header
  if (head != 0){
    uint64_t *head_payload = get_payload(head);
    
    head_payload[0] = (uint64_t) header;
  }
  
  heads[idx] = (uint64_t *) header;
}

// removes the block at header from the free list
// takes as input the header pointer of a block
// returns nothing
// payload[0] points to the previous block's header, payload[1] the next block's header
void list_remove(void *header){
  if (header == NULL) {return;}
  
  // calculate segregated list to remove from
  size_t size = *(uint64_t *) header & ~3;
  uint64_t idx = size_to_list(size);

  uint64_t *payload = (uint64_t*) get_payload(header);

  void *prev = (void *) payload[0];
  void *next = (void *) payload[1];
  
  // link previous block to the next block
  if (prev != 0) {
    uint64_t *prev_payload = (uint64_t *) get_payload(prev);
    prev_payload[1] = (uint64_t) next;
  } else {
    heads[idx] = (uint64_t *) next; // the removed block was the head
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
// takes as input the header pointer of a block and that block's size
// returns the header of the merged block
void *merge(void *header, size_t block_size){

  if (header == NULL) {return NULL;}
  

  // right neighbor header
  uint64_t *right_neighbor = (uint64_t*) ((char*) header + block_size);
  
  uint64_t left_allocated = (*(uint64_t *) header & 2) / 2;
  uint64_t right_allocated = *right_neighbor & 1;
  
  void *new_header = header;
  size_t new_size = block_size;
  
  // merge the free neighbors
  
  if (!right_allocated) {
    new_size += *right_neighbor & ~3;
    list_remove(right_neighbor);
  }
  
  if (!left_allocated) {
    size_t l_size = *((uint64_t *) header - 1) & ~3;
    new_header = (char *) header - l_size; // shift the pointer to the left neighbor's header
    list_remove(new_header);
    new_size += l_size;
  }
  
  set_header (new_header, new_size, 0);
  set_footer(new_header, new_size, 0);
  
  return new_header;
}

// gets the header for a payload; breaks convention here because it's for a specific use case in free()
// takes as input a payload pointer
// returns the header pointer for the block containing payload
void *get_header(void *payload) {
  if (payload == NULL) {return NULL;}
  
  void *header = (void *) ((char *) payload - 8);
  
  return header;
}

/*
 * free
 */
void free(void* payload)
{
    if (payload == NULL) {return;}
    
    // free the block
    uint64_t *header = (uint64_t *) get_header(payload);
    
    size_t block_size = *header & ~3;

    // clear allocated bit
    *header = *header & ~1ULL;
    
    // merge any free neighbors
    void *merged = merge(header, block_size);
    
    if (merged == NULL) return;
    
    size_t merged_size = *(uint64_t *) merged & ~3;
    update_right_header(merged, merged_size, 0);
    list_insert(merged);
}
/*
 * realloc
 */
void* realloc(void* oldptr, size_t size)
{

    // resizing from a null pointer behaves as a malloc
    if (oldptr == NULL) {
      void *new_block = malloc(size);
      
      if (new_block == NULL) {return NULL;}
      
      return new_block;
    }
    
    
    // resizing to 0 bytes frees the block
    if (size == 0) {
      free(oldptr);
      oldptr = NULL;
      
      return NULL;
    }
    
    
    void *new_block = malloc(size);
    
    if (new_block == NULL) return NULL;
    
    size_t copy_size;
    
    size_t old_payload_size = (*(uint64_t *) get_header(oldptr) & ~3) - 8; 
    
    // make sure only the necessary bytes are copied
    if (size < old_payload_size) {
      copy_size = size;
    } else {
      copy_size = old_payload_size;
    }
    
    new_block = memcpy(new_block, oldptr, copy_size);
    
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

#endif // DEBUG
    return true;
}