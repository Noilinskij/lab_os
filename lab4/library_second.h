#ifndef HEADER_SECOND
#define HEADER_SECOND

#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Block{
    size_t size;
    // 0 - свободен, 1 - занят
    int status;
    struct Block* next;
} Block;

typedef struct Allocator{
    size_t size;
    void* memory;
    Block* list;
} Allocator;

#endif 