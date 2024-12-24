#include <sys/mman.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct Allocator Allocator;

// пользовательское api
// static привязывает allocator_{func} только для этого файла
static Allocator* (*allocator_create)(void* const memory, const size_t size);
static void* (*allocator_alloc)(Allocator* const allocator, const size_t size);
static void (*allocator_destroy)(Allocator* const allocator);
static void (*allocator_free)(Allocator* const allocator, void* const memory);

Allocator* allocator_create_stub(void* const memory, const size_t size) {
    (void) memory;
    (void) size;
    return NULL;
}

void* allocator_alloc_stub(Allocator* const allocator, const size_t size) {
    (void) allocator;
    (void) size;
    return NULL;
}

void allocator_destroy_stub(Allocator* const allocator) {
    (void) allocator;
}

void allocator_free_stub(Allocator* allocator, void* const memory) {
    (void) memory;
    (void) allocator;
}



int main(int argc, char** argv) {
    void* library = NULL;
    char ans[100];
    if (argc < 2) {
        library = dlopen("./library_first.so", RTLD_LOCAL | RTLD_NOW);
    }
    else {
        library = dlopen(argv[1], RTLD_LOCAL | RTLD_NOW);
    }

    if (library == NULL) {
        allocator_create = allocator_create_stub;
        allocator_destroy = allocator_destroy_stub;
        allocator_free = allocator_free_stub;
        allocator_alloc = allocator_alloc_stub;
    }
    else {
        allocator_create = dlsym(library, "allocator_create");
        if (allocator_create == NULL) {
            allocator_create = allocator_create_stub;
        }
        allocator_destroy = dlsym(library, "allocator_destroy");
        if (allocator_create == NULL) {
            allocator_destroy = allocator_destroy_stub;
        }
        allocator_free = dlsym(library, "allocator_free");
        if (allocator_create == NULL) {
            allocator_free = allocator_free_stub;
        }
        allocator_alloc = dlsym(library, "allocator_alloc");
        if (allocator_create == NULL) {
            allocator_alloc = allocator_alloc_stub;
        }
    }
    void* memory = mmap(0, 1024 + 24, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    Allocator* allocator = allocator_create(memory, 1024 + 24);
    if (allocator == NULL) {
        write(1, "create allocator fail", sizeof("create allocator fail"));
        exit(1);
    }
    int* x = allocator_alloc(allocator, sizeof(int));
    if (x == NULL) {
        write(1, "alloc fail", sizeof("alloc fail"));
        exit(1);
    }
    *x = 4;
    *x = 8;
    snprintf(ans, sizeof(ans), "%d ", *x);
    write(1, ans, 2);
    allocator_free(allocator, x);
    char* str = allocator_alloc(allocator, 256);
    if (str == NULL) {
        write(1, "alloc fail", sizeof("alloc fail"));
        exit(1);
    }
    str[0] = 'h';
    str[1] = 'e';
    str[2] = 'l';
    str[3] = 'l';
    str[4] = 'o';
    str[5] = 0;
    write(1, str, sizeof(str));
    allocator_destroy(allocator);
    if (library != NULL) {
        dlclose(library);
    }
    return 0;

}