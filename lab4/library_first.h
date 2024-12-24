#ifndef HEADER_FIRST
#define HEADER_FIRST

#include <stdlib.h>
#include <sys/mman.h>

typedef struct Block {
    size_t size;
    // 1 - занят, 0 - нет
    int status;
    struct Block* next;
    struct Block* prev;
} Block;


typedef struct Allocator {
    size_t size;
    void* memory;
    Block* list;

} Allocator;

#endif