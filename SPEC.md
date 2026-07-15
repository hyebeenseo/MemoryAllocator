# Memory Allocator v.1

## Description
Implement a memory allocator that manages a fixed-size memory arena obtained from the operating system using `mmap()`.

The allocator provides `allocate()` and `deallocate()` operations for dynamic memory allocation and deallocation.

## Semantics
- `allocate(N)`, where N > 0: Returns a pointer to a memory region of at least N usable bytes if the allocation succeeds. Returns `NULL` if the allocation cannot be satisfied.

- `allocate(0)`: Return `NULL`.

- `deallocate(p)`, where p is a pointer previously returned by `allocate()` and has not yet been deallocated: Releases the corresponding allocated region, making it available for future allocations.

- `deallocate(NULL)`: Has no effect.

- `deallocate(p)`. where p is an interior pointer into an allocated region: The behavior is undefined.

- `deallocate(p)`, where p was not returned by `allocate()`: The behavior is undefined.

- `deallocator(p)`, where p has already been deallocated: The behavior is undefined.

## Guarantees
- A successful `allocate(N)` returns a memory region containing at least N usable bytes.
- The usable region of any two live allocations do **not overlap.**
- Memory released by `deallocate()` may be reused by subsequent allocations.
- The contents of a deallocated memory region are not preserved.


## Constraints

## Error Handlings

## Comletion Criteria
