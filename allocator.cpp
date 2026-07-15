#include "allocator.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define HEAP_SIZE 4096

/* NOTE: block header size = RESERVED size of allocated node of the free list*/
typedef struct {
    size_t     size;
} BlockHeader;

typedef struct ListNode_ {
    size_t     size;
    ListNode_* next;
} ListNode;

typedef struct FreeList_ {
    int num;
    ListNode* head;
} FreeList;

/*=========== headers ============*/
void* get_heap_arena(size_t size);
FreeList* initialize_free_list();
size_t get_needed_block_size(size_t size);
void* first_fit(size_t requested_size);
void* get_available_block(size_t requested_size);
BlockHeader* split(ListNode* ptr, ListNode* pred, size_t original_size, size_t needed_size);
BlockHeader* fill_block_header(BlockHeader* block_header, size_t reserved_size);
void coalesce(ListNode* ptr, ListNode* pred, ListNode* succ);
void split_free_list(ListNode* free_addr, ListNode* pred, void* front_next, ListNode* rear);
void add_free_list(ListNode* ptr);
void print_free_list();
void rm_free_list(ListNode* ptr, ListNode* pred);
void* allocate(size_t size);
void deallocate(void* ptr);

/*================================*/

/*======== global variables =======*/

FreeList* free_list = initialize_free_list();

/*=================================*/

/* Obtain heap memory arena to manage, using mmap() syscall */
void* get_heap_arena(size_t size) {
    void* ptr = mmap(NULL, size, PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    return ptr;
}

FreeList* initialize_free_list() {

    ListNode* initial_node = (ListNode*)get_heap_arena(HEAP_SIZE);
    initial_node->size = HEAP_SIZE;
    initial_node->next = NULL;

    FreeList* free_list = (FreeList*)get_heap_arena(sizeof(free_list));
    free_list->num = 1;
    free_list->head = initial_node;

    //printf("--Heap Memory Initilized--\n Available: %d bytes\nstarting at %p\n-----------------------\n", initial_node->size, initial_node);
    return free_list;
}

// NOTE: ALIGNMENT NEEDED --> size must be multiple of 8
size_t get_needed_block_size(size_t size) {

    size_t alignment = alignof(ListNode);
    size_t block_size = sizeof(BlockHeader) + size;
    if (block_size > sizeof(ListNode)) {
        block_size = (block_size + alignment - 1) / alignment * alignment;
        return block_size;
    }
    else return sizeof(ListNode);
}

// ptr: free list에 있던 node pointer
// NOTE: free list node header size > block header size
BlockHeader* split(ListNode* ptr, ListNode* pred, size_t original_size, size_t needed_size) {
    
    void* front_next = ptr->next;

    ListNode* new_node = (ListNode*)((char*)ptr + needed_size);

    new_node->size = original_size - needed_size;
    new_node->next = NULL;

    split_free_list(ptr, pred, front_next, new_node);

    BlockHeader* ret = fill_block_header((BlockHeader*)ptr, needed_size);

    //printf("Allocated: %p\n", ret);

    return ret;
}


/* Use free_list (global) */
/* Returns (requested size + header size) heap pointer if succeeded,
   Returns NULL otherwise */
void* first_fit(size_t needed_size) {
    BlockHeader* ret = NULL;
    ListNode* pred = NULL;
    ListNode* block_ptr = free_list->head;
    while (block_ptr != NULL) {
        if (block_ptr->size >= needed_size + sizeof(ListNode)) {
            ret = split(block_ptr, pred, block_ptr->size, needed_size);           
            break;
        } else if (block_ptr->size >= needed_size) {
            size_t block_size = block_ptr->size;
            ret = fill_block_header((BlockHeader*)block_ptr, block_size);
            rm_free_list((ListNode*)block_ptr, pred);
        } else {
            pred = block_ptr;
            block_ptr = block_ptr->next;
        }
    }

    /* the size info of free list is reachable in this function,
        therefore we shoule call fill_block_header here*/
    return ret;
}

BlockHeader* fill_block_header(BlockHeader* block_header, size_t size) {
    block_header->size = size;
    return block_header;
}

/* Returns an available pointer with size user requested if succeeded,
   Returns NULL otherwise */
void* get_available_block(size_t requested_size) {

    // Case where requested size is zero
    if (requested_size == 0) return NULL;

    size_t needed_size = get_needed_block_size(requested_size);
    void* hptr = first_fit(needed_size);
    if (hptr == NULL) {
        return NULL;
    } else {
        return (BlockHeader*)hptr + 1;
    }
}

void split_free_list(ListNode* free_addr, ListNode* pred, void* front_next, ListNode* rear) {
    //printf("\n--split_free_list processing ... \n");

    if (free_addr == free_list->head) {
        //printf("\nhead, address: %p, next: %p\n", free_addr, front_next);
        free_list->head = rear;
    }

    
    //printf("front: %p, next of front: %p\n", free_addr, front_next);
    //printf("rear: %p, next of rear: %p\n", rear, rear->next);
    //print_free_list();


    //printf("\nrear->next = front->next에서 front: %p, front->next: %p\n", free_addr, front_next);
    if (pred != NULL) {
        pred->next = rear;
    }
    rear->next = (ListNode*)front_next;
    return;
}


/* BUG: when ptr is the least address, cur is not succ of ptr */
void add_free_list(ListNode* ptr) {
    ListNode* cur = free_list->head;
    ListNode* pred = NULL;
    //ListNode* dpred = NULL;

    while (cur != NULL && ptr > cur) {
        //dpred = pred;
        pred = cur;
        cur = cur->next;
    }
    if (pred == NULL) {        // ptr is least address (will become head)
        ptr->next = cur;
        free_list->head = ptr;
        coalesce(ptr, NULL, ptr->next);
    } else {
        ptr->next = cur;
        pred->next = ptr;
        coalesce(ptr, pred, ptr->next);
    }
    return;
}

void rm_free_list(ListNode* ptr, ListNode* pred) {
    // When ptr given node is head of the free list
    if (free_list->head == ptr) {
        free_list->head = ptr->next;
    }
    if (pred != NULL) { 
        pred->next = ptr->next;
    }
    return;
}


/* Use free_list (global) */
/* INPUT ptr: pointer to a memory that is freed just before */
void coalesce(ListNode* ptr, ListNode* pred, ListNode* succ) {
    ListNode* free_ptr_cur = free_list->head;

    if (pred != NULL) {
        if ((char*)pred + pred->size == (char*)ptr) {
            pred->size = pred->size + ptr->size;
            pred->next = ptr->next;

            ptr = pred;
            //printf("Coalesced - pred + cur\naddress of cur: %p\n", ptr);
        }
    }
    if (ptr != NULL) {
        if ((char*)ptr + ptr->size == (char*)succ) {
            ptr->size = ptr->size + succ->size;
            ptr->next = succ->next;

            succ = ptr;
            //printf("Coalesced - cur + next\naddress of cur: %p\n", ptr);
        }
    }

    return;
    /* CONSIDER: when the list head is included:
        second case matters in that case, and add_free_list handles list.
    */

    /* CASE 1. coalesced: prev+ptr, succ.
       CASE 2. not coalesced: prev, ptr, succ 
       Generalization: ptr <- pred */

    
}

/* Block level --> input: user knowing (user using) pointer, successing header*/
void free_block(void* ptr) {

    // Case where ptr is NULL
    if (ptr == NULL) return;

    BlockHeader* hptr = (BlockHeader*) ptr - 1;
    size_t block_size = hptr->size;

    /* Makes given space into free list node form */
    ListNode* nptr = (ListNode*)hptr;
    nptr->size = block_size;

    /* Adds it to free_list by calling add_free_list */
    add_free_list(nptr);

    return;
}


void print_free_list() {
    int idx = 0;
    printf("\nState of Free List\n");
    ListNode* pnode = free_list->head;
    while (pnode != NULL) {
        printf("node #  : %d\n", idx);
        printf("address : %p\n", pnode);
        printf("size    : %zu\n", pnode->size);
        printf("next    : %p\n", pnode->next);
        
        idx++;
        pnode = pnode->next;
    }
    return;
}



/* ======== API functions =========*/

void* allocate(size_t requested_size) {

    void* ret = get_available_block(requested_size);

    //printf("--Allocation--\nrequested size: %d\n", requested_size);

    //printf("\nAfter Allocation: Free list state\n");
    print_free_list();
    //printf("\n");

    return ret;

}

void deallocate(void* ptr) {
    free_block(ptr);

    //printf("--Deallocation--\naddress: %p\n", ptr);

    //printf("\nAfter Deallocation: Free list state\n");
    print_free_list();
    //printf("\n");
    return;
}

/*=================================*/

/*
int main() {
    printf("main function started\n");
    char* str1 = (char*)allocate(20);
    strcpy(str1, "hello world");

    char* str2 = (char*)allocate(20);
    strcpy(str2, "yayayayayayayay");

    printf("%s\n", str1);
    printf("%s\n", str2);
    deallocate(str1);
    deallocate(str2);

    int* num1 = (int*)allocate(sizeof(int));
    *num1 = 29292;

    printf("???%d\n", *num1);
    deallocate(num1);

    return 0;
}
*/