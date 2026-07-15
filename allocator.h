#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

void *allocate(size_t size);
void deallocate(void *ptr);

#endif
