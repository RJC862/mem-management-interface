# Overview
This is a Memory Management Interface I wrote for my Operating Systems course. See project details & description below.    
Source code in mm.c  

# Implementation Details  
You can also find this at the top of the source code  
 * custom helper functions:  
 *   get_header(), get_footer(), get_payload(),  
 *   set_header(), set_footer(),  
 *   find_free_block(), split_block()  
 *   list_insert(), list_remove()  
 *   merge()  
 * 
 * High Level Implementation Description:  
 *    
 *  The project implements malloc() and free(), as well as realloc() and calloc().  
 *   
 *  A doubly linked list is used to track free blocks, and splitting and coalescing are utilized to minimize wasted memory.  
 *  
 *  void *malloc(size_t size) - allocates size bytes of memory requested by the caller.  
 *  The mechanism behind malloc() works as such:   
 *    1. A free list is searched for a free block of suitable size. In this case, the suitable size is at least size + 16 bytes, because we also require space for the header and footer.  
 *    2. If a free block of suitable size is found, but its size is at least 32 bytes larger than the smallest suitable size, we split it and return the remaining chunk back to the free list. Otherwise do nothing.  
 *    3. The free block is removed from the list, its header and footer are set appropriately, and the payload is returned to the caller.  
 *    
 *  void free(void *payload) - deallocates the block of memory containing payload.  
 *  The mechanism behind free() works as such:  
 *    1. First, the allocated bit is set to 0 for the header and footer of the block containing payload. This effectively deallocated the memory block.  
 *    2. Coalesce using merge(). Coalescing amends external fragmentation in the heap.  
 *    3. Coalesced/Merged block is then added back to the free list.  
 *   
 *  void* realloc(void* oldptr, size_t size) - resizes the memory chunk at oldptr to size bytes.  
 *  The mechanism behind realloc() works as such:  
 *    1. Call malloc(), for a chunk of memory of size bytes.  
 *    2. Copy the bytes at oldptr to the new malloc'd block.  
 *    3. Return the new malloc'd payload.   
 *  
 *  Note: The convention is usually to use header pointers as a base for pointer arithmetic.  
 *    This is because free blocks point to each other's headers in the linked list,   
 *    and the linked list is the basis of memory management.  
 *  
 *  Custom helper functions are documented with in-function comments.   
 *    
 */  

# Project 3: Malloclab Dynamic Storage Allocator

## Release Date: 10/10/2026, 6 PM Checkpoint 1 Due Date: 10/20/2026, 6 PM Checkpoint 2 Due Date: 10/30/2026, 6 PM

**Please read this document carefully before you start coding.**

## Getting Started and Repository Rules

You may work alone or with one partner.

Clone your repository onto a Linux machine:

```bash
git clone <your-repository-url>
cd <repository-directory>
```

Use the CSE Linux machines (e.g., the W135 Westgate lab; access: https://www.eecs.psu.edu/cse-student-lab-access/index.aspx).

Do not seek help from anyone outside your group. Acknowledge and explain any online resources you used, including AI tools.

Some important notes:

* Please do not commit build outputs (binaries, core dumps, editor backups).
* Both partners are expected to contribute.
* A fresh clone of your repository must build with `make`.

---

## Introduction

In this lab, you will write your own `malloc`, `free`, and `realloc`. Your allocator should be correct, use little memory, and be fast. Please read this document together with the tutorial PDF on Canvas.

You only need to change `mm.c`; changes to other files are ignored when grading. You will implement the following functions:
- `bool mm_init(void)`
- `void* malloc(size_t size)`
- `void free(void* ptr)`
- `void* realloc(void* oldptr, size_t size)`
- `bool mm_checkheap(int line_number)` (optional, see the last section of this document)

`calloc` is already written for you in the starter code. You may add static helper functions and structs.

## Description of the dynamic memory allocator functions

- `mm_init`: The driver calls it before every trace, before any other call. Please set up your initial heap and reset all your global variables here, and return true on success or false on failure. Do not call it yourself.

- `malloc`: Returns a 16-byte aligned pointer to the payload (the usable bytes after the header), at least `size` bytes long, inside the heap, not overlapping any other allocated block. Returns NULL if `mm_sbrk` cannot extend the heap.

- `free`: Frees the block at `ptr`. `ptr` is the payload pointer that `malloc` returned, not the header. It always comes from an earlier `malloc`, `calloc`, or `realloc` and has not been freed yet. `free(NULL)` does nothing.

- `realloc`: Returns a pointer to at least `size` bytes.
    - If `ptr` is NULL, it is the same as `malloc(size)`.
    - If `size` is 0, it is the same as `free(ptr)`.
    - Otherwise, it resizes the block at `ptr`. The new block may or may not be at the same address. Its contents stay the same up to the smaller of the old and new sizes; the rest is uninitialized.

These match the libc functions; see `man malloc` for details.

## Support routines

`memlib.c` simulates the memory system. You can call these functions:

- `void* mm_sbrk(int incr)`: Extends the heap by `incr` bytes (`incr` > 0) and returns a pointer to the first new byte. Please use it instead of `sbrk`.
- `void* mm_heap_lo(void)`: Returns a pointer to the first byte of the heap.
- `void* mm_heap_hi(void)`: Returns a pointer to the last byte of the heap.
- `size_t mm_heapsize(void)`: Returns the heap size in bytes.
- `size_t mm_pagesize(void)`: Returns the page size in bytes (4K on Linux).
- `void* memset(void* ptr, int value, size_t n)`: Sets the first `n` bytes at `ptr` to `value`.
- `void* memcpy(void* dst, const void* src, size_t n)`: Copies `n` bytes from `src` to `dst`.

Only `mm_sbrk` is required; the others are there if you need them.

## Programming Rules

- Please do not change any of the interfaces in `mm.c`.
- Do not call any memory-management library functions or system calls, such as `sbrk`, `brk`, or the libc `malloc`, `calloc`, `free`, and `realloc`. Use `mm_sbrk` instead.
- Your code must work on 64-bit machines; sizes and offsets need 8 bytes.
- Do not use macros with parameters; use static functions instead, and the compiler will inline them.
- You may use at most 128 bytes of global space (arrays, structs, etc.). Please put anything larger in the heap.
- Do not copy code from the textbook.

## Evaluation and testing your code

You will receive zero points if:
- You violate the academic integrity policy (sanctions can be greater than just a 0 for the assignment)
- You don't show your partial work by periodically adding, committing, and pushing your code to GitHub
- You break any of the programming rules
- Your code does not compile/build
- Your code crashes the grading script

Otherwise, each checkpoint is scored on space utilization (60%) and throughput (40%):
- Space utilization: the peak ratio between the memory the driver has allocated and not yet freed, and the size of your heap.
- Throughput: the average number of operations per second. It varies between runs and machines, so your local score is only an estimate.

Each metric has a min and a max target: below the min you get 0, above the max you get full points, and in between the points scale linearly. If any trace fails, or either metric is below a required minimum, the whole checkpoint is 0. The targets are:

| | Checkpoint 1 | Checkpoint 2 |
|---|---|---|
| Utilization: required / min / max | 30% / 40% / 50% | 40% / 50% / 65% |
| Throughput: required / min / max | 2% / 0% / 5% | 2% / 30% / 90% |

Throughput targets are percentages of the benchmark: the speed of a reference allocator on the same machine, measured by the calibration below.

- **[40 pts] Checkpoint 1:** **Recommended: Steps 1 to 4. An explicit free list (Step 3) earns most of the credit; adding coalescing (Step 4) earns full credit.**
- **[60 pts] Checkpoint 2 (final submission):** **Recommended: Steps 5 and 6, segregated free lists with the footer optimization.**

The recommended designs are suggestions, not requirements: any design that meets the targets earns the points.

**IMPORTANT:** You may develop on any Linux machine, but please run your final tests on a CSE lab machine. The first `make test` on a machine runs a calibration (a few minutes) and saves it in `throughputs.txt`. On a laptop, please stay plugged in with high performance settings; if the calibration ran on battery, delete `throughputs.txt` and run it again.

Please run `make` after every change. Run all traces with `make test`, or one with `./mdriver -f traces/<file>.rep`; the `-short.rep` traces are small and good for your first debugging. Add `-v` or `-V` for more output, and see `./mdriver -h` for other options.

A trace file starts with four header numbers, then has one call per line:

```
a 0 512    p0 = malloc(512)
a 1 128    p1 = malloc(128)
r 0 640    p0 = realloc(p0, 640)
f 1        free(p1)
```

`make test` prints one line per trace, then the score of each checkpoint. A shortened example:

```
  valid    util     ops   msecs    Kops  trace
   yes    42.9%       5     0.000  27064 ./traces/syn-example-short.rep
 ...
 * yes    66.0%   80000    25.337   3157 ./traces/syn-struct.rep

***Checkpoint 1 perf index = 24.0 (util) + 16.0 (thru) = 40.0/40.0***
Average utilization = 54.8%. Average throughput = 9230 Kops/sec
Utilization targets: min=40.0%, max=50.0%
Throughput targets: min=0, max=1148, benchmark=22956
 ...
Score: Checkpoint 1: 40 / 40, Checkpoint 2: 16 / 60
```

- `valid` is `yes` if the trace ran correctly. `util` and `Kops` (thousand operations per second) are for that trace alone; only the traces marked `*` count toward the score.
- Each checkpoint block shows your averages, the points they earn, and the targets. Throughput targets are in Kops, computed from the benchmark of your machine.
- The last line is your score.

## Recommended steps

Build your allocator one step at a time and commit after each step works. Every step passes the same correctness tests as the one before it. What improves is utilization and throughput.

1. **Allocate with `mm_sbrk` only.** Every `malloc` gets a new block at the end of the heap; `free` does nothing. Give every block an 8-byte header and an 8-byte footer that hold the block size and an allocated bit, and round each request up to a multiple of 16 with the `align` helper. Start the heap with 8 bytes of padding so that every payload is 16-byte aligned. This passes the correctness tests, but its utilization is too low for Checkpoint 1.
2. **Implement `free`.** Clear the allocated bit in the block's header and footer. Nothing is reused yet, but the heap now knows which blocks are free.
3. **Explicit free list with splitting.** Keep the free blocks in a doubly-linked list: store the `prev` and `next` pointers inside the free block's payload, and keep one global pointer to the head. `malloc` takes the first free block that is big enough and calls `mm_sbrk` only when nothing fits; `free` inserts the block into the list. If the block is bigger than needed and the rest is at least 32 bytes, split it and put the rest back into the list. This earns most of the Checkpoint 1 credit.
4. **Coalesce.** When you free a block, merge it with its free neighbors; the left block's footer tells you where that block starts. A free neighbor is already in the list, so remove it before merging, then insert the merged block. This earns full credit on Checkpoint 1.
5. **Segregated free lists.** Keep one free list per size class, for example one per power of two, with the last list taking everything larger. Every place that touches the free list now has to pick the list by block size: `malloc` searches its own size class first and then the larger ones, `free` and splitting insert into the class of the block's new size, and coalescing removes each free neighbor from its own class before inserting the merged block. A small helper that maps a size to its list index is useful here.
6. **Footer optimization.** Allocated blocks drop their footer, and those 8 bytes become payload, so a block is now the header plus the payload, rounded up to 16. Use a second header bit to record whether the previous block is allocated, and update that bit in the next block whenever you allocate or free a block. Free blocks still keep their footer, because coalescing reads it.

## Handin

We use GitHub for submissions. Please include your GitHub username when you ask questions on Canvas.

For each checkpoint, please paste the full commit hash you want graded into that checkpoint's Canvas submission.

## Hints

- *Draw pictures.* Draw the heap with addresses and sizes before you write the code.

- *Keep your code modular.* Put pointer arithmetic and bit manipulation in small static helper functions; the compiler will inline them. Keep the linked list code in its own insert and remove functions, so that splitting and coalescing can reuse it without worrying about list bugs. If you find yourself copying code between functions, write a function instead: duplicated code means duplicated bugs.

- *Know what pointer arithmetic does.* Adding `n` to a pointer moves it by `n` times the size of the type it points to. If `p` is a `char*`, `p + 8` moves 8 bytes; if `p` is a `uint32_t*`, it moves 32 bytes; if `p` is a `uint64_t*`, it moves 64 bytes. Do byte offsets on a `char*`, and cast only when you read or write a header, e.g. `*(uint64_t*)((char*)bp + size)`. Headers and footers are 8 bytes, so read and write them as `uint64_t`, not `uint32_t` or `int`. Name each pointer by what it points to: the block (header) or the payload.

- *Document your design.* Describe your design at the top of `mm.c`.

- *Use gdb.* Build with `make debug` and run `gdb mdriver`; watchpoints help find who overwrites a header.

## Heap consistency checker

**The heap checker is not graded, but we strongly recommend writing one.**

It scans your heap and checks invariants, so it catches a bug where it happens, not many calls later. For example:
- Is every block in the free list marked as free?
- Are there any contiguous free blocks that somehow escaped coalescing?
- Is every free block actually in the free list?
- Do the pointers in the free list point to valid free blocks?
- Do any allocated blocks overlap?
- Do the pointers in a heap block point to valid heap addresses?

Return true if the heap is consistent and false otherwise. Use `dbg_printf` to print errors; to enable it, uncomment `#define DEBUG`. Call it as `mm_checkheap(__LINE__)` to see which line found the problem.
