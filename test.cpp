#include "allocator.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_basic_allocation(void)
{
    printf("[TEST] basic allocation\n");

    int *p = (int*)allocate(sizeof(int));
    assert(p != NULL);

    *p = 42;
    assert(*p == 42);

    deallocate(p);
}

static void test_multiple_allocations(void)
{
    printf("[TEST] multiple allocations\n");

    char *a = (char*)allocate(64);
    char *b = (char*)allocate(128);
    char *c = (char*)allocate(32);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);

    memset(a, 'A', 64);
    memset(b, 'B', 128);
    memset(c, 'C', 32);

    for (int i = 0; i < 64; i++)
        assert(a[i] == 'A');

    for (int i = 0; i < 128; i++)
        assert(b[i] == 'B');

    for (int i = 0; i < 32; i++)
        assert(c[i] == 'C');

    deallocate(a);
    deallocate(b);
    deallocate(c);
}

static void test_reuse_freed_block(void)
{
    printf("[TEST] reuse freed block\n");

    void *a = allocate(128);
    void *b = allocate(128);
    void *c = allocate(128);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);

    deallocate(b);

    void *d = allocate(64);

    assert(d != NULL);

    /*
     * Under first-fit, d should reuse the block released by b.
     */
    assert(d == b);

    deallocate(a);
    deallocate(d);
    deallocate(c);
}

static void test_coalescing(void)
{
    printf("[TEST] coalescing\n");

    void *a = allocate(128);
    void *b = allocate(128);
    void *c = allocate(128);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);

    deallocate(a);
    deallocate(b);

    /*
     * This allocation should require the adjacent free blocks
     * a and b to be coalesced.
     */
    void *d = allocate(200);

    assert(d != NULL);
    assert(d == a);

    deallocate(d);
    deallocate(c);
}

static void test_fragmentation(void)
{
    printf("[TEST] fragmentation\n");

    void *a = allocate(100);
    void *b = allocate(100);
    void *c = allocate(100);
    void *d = allocate(100);

    assert(a != NULL);
    assert(b != NULL);
    assert(c != NULL);
    assert(d != NULL);

    deallocate(b);
    deallocate(d);

    /*
     * Two separate 100-byte free regions must not be treated
     * as one contiguous 200-byte region.
     */
    void *e = allocate(150);

    assert(e != b);
    
    deallocate(a);
    deallocate(c);

    if (e != NULL)
        deallocate(e);
}

static void test_zero_allocation(void)
{
    printf("[TEST] allocate(0)\n");

    assert(allocate(0) == NULL);
}

static void test_null_deallocation(void)
{
    printf("[TEST] deallocate(NULL)\n");

    deallocate(NULL);
}

static void test_allocation_failure(void)
{
    printf("[TEST] allocation failure\n");

    /*
     * Arena size is 4096 bytes, so this cannot succeed.
     */
    void *p = allocate(4096);

    assert(p == NULL);
}

static void test_many_small_allocations(void)
{
    printf("[TEST] many small allocations\n");

    void *blocks[32];

    for (int i = 0; i < 32; i++) {
        blocks[i] = allocate(32);
        assert(blocks[i] != NULL);
    }

    for (int i = 0; i < 32; i += 2)
        deallocate(blocks[i]);

    for (int i = 0; i < 16; i++) {
        void *p = allocate(16);
        assert(p != NULL);
        deallocate(p);
    }

    for (int i = 1; i < 32; i += 2)
        deallocate(blocks[i]);
}

int main(void)
{
    
    test_basic_allocation();
    
    test_multiple_allocations();
    test_reuse_freed_block();
    test_coalescing();
    test_fragmentation();
    test_zero_allocation();
    test_null_deallocation();
    test_allocation_failure();
    
    test_many_small_allocations();

    printf("\nAll tests passed.\n");

    return 0;
}