# Memory Allocator v.1

A Simple free-list based memory allocator implemented in C.

This project was built from scratch to understand dynamic memory management, free-list repersentations,
block splitting, coalescing, and memory alignment.

## Features

- Heap arena initialized with 'mmap()'
- Dynamic memory allocation and deallocation
- First-fit allocation policy
- Address-ordered free list
- Block splitting
- Adjacent free block coalescing
- Memory alignment handling

## Design
### Block Representation
Allocated blocks and free blocks use different metadata representations.

Allocated block:
[ BlockHeader ][ payload ]

Free block:
[ ListNode][ free space ]

Free-list metadata is embedded directly inside free blocks.

To ensure that every allocated block can later be represented as a free block,
every block has a minimum size of `sizeof(ListNode)`.

### Allocation

Allocation consists of three steps:
1. Find a sufficiently large free block using first-fit.
2. Split the block if the remaining space is larger enough to represent a free block.
3. Otherwise, consume the entire free block.

A block is split only if:
remaining_size >= sizeof(ListNode)

### Deallocation and Coalescing

Deallocated blocks are inserted into the free list in address order.

Because the free list is address-ordered, adjacent free blocks can be detected
using the predecessor and successor of the inserted block.

Adjacent blocks are coalesced to reduce external fragmentation.

## Important Invariants

- Every block is large enough to represent a free block.
- Every block starts at a properly aligned address.
- Every free block appears exactly once in the free list.
- Every free-list node represents a valid free block.
- Free-list pointers are either `NULL` or point to valid free blocks.
- Live allocations do not overlap.


## Build
```bash
g++ allocator.cpp test.cpp -o allocator_test
```

## Usage
```C
#include "allocator.h"

void *ptr = allocator(100);

if (ptr != NULL) {
  /* use memory */
}

deallocate(ptr);
```

## Limitations
This is a learning-oriented allocator and is ot intended for production use.

Current limitations include:
- Fixed size heap arena
- Limited invalid-free detection
- No thread safety
- No heap growth
- No advanced allocation policies or size classes

## What I Learned


## Future Work

- Invalid-free detection and error handling
- Heap growth
- Additional allocation policies
- Garbage collection experiments
