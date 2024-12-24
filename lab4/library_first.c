#include "library_first.h"


Allocator* allocator_create(void* const memory, const size_t size) {
    if (memory == NULL || size < sizeof(Allocator) + sizeof(Block)) {
        return NULL;
    }
    // памятью для нашего аллокатора станет начало данной нам памяти
    Allocator* allocator = (Allocator*) memory;
    allocator->memory = memory;
    allocator->size = size;

    Block* first = (Block*) (memory + sizeof(Allocator));
    first->size = size - sizeof(Allocator) - sizeof(Block);
    first->status = 0;
    first->next = NULL;
    first->prev = NULL;
    allocator->list = first;
    return allocator;
}

void* allocator_alloc(Allocator* const allocator, const size_t size) {
    if (allocator == NULL || size <= 0) {
        return NULL;
    }
    Block* current = allocator->list;
    Block* best = NULL;
    // ищем наиболее подходящий блок
    while(current) {
        if (current->size >= size && current->status == 0) {
            if (best == NULL) {
                best = current;
            }
            else if(current->size - size < best->size - size) {
                best = current;
            }
        }
        current = current->next;
    }
    if (best == NULL) {
        return NULL;
    }

    // если новый выделенный блок больше нужного и есть место под еще один новый блок, 
    // то создадим еще один
    if (best->size > size + sizeof(Block)) {
        Block* new = (Block*) ((void*) best + size + sizeof(Block));
        new->size = best->size - sizeof(Block) - size;
        new->status = 0;
        new->next = best->next;
        if (best->next) {
            best->next->prev = new;
        }
        new->prev = best;
        best->next = new;
        best->size = size;
    }

    best->status = 1;
    // важно соблюдать арифметику указателей
    return (void*) best + sizeof(Block);


}

void allocator_destroy(Allocator* const allocator) {
    if (allocator != NULL) {
        munmap(allocator->memory, allocator->size);
    }
}

void allocator_free(Allocator* const allocator, void* const memory) {
    if (allocator == NULL || memory == NULL) {
        return;
    }
    Block* current = (Block*) (memory - sizeof(Block));
    current->status = 0;
    // если предыдущий блок свободный, то объединяем 2 блока
    if (current->prev != NULL && current->prev->status == 0) {
        // пропадает именно текущий, тк у объединенного тогда память под структуру будет в начале блока
        current->prev->size = current->prev->size + current->size + sizeof(Block);
        current->prev->next = current->next;
        if (current->next) {
            current->next->prev = current->prev;
        }
        current = current->prev;
    }

    if (current->next != NULL && current->next->status == 0) {
        current->size = current->size + current->next->size + sizeof(Block);
        current->next = current->next->next;
        if(current->next) {
            current->next->prev = current;
        }
    }


}


// тесты:
// проверка, что sizeof(Allocator) + sizeof(Block) + sizeof(int) выделят ровно под одно число

// проверка, что sizeof(Allocator) + sizeof(Block) + 3 не выделят память под число, но выделят под char

// проверка, что sizeof(Allocator) + sizeof(Block) * 2 + 5 выделят память только под число и char

// проверка, что sizeof(Allocator) + sizeof(Block) + 4 корректно выделит и освободит память под char, а потом выделит под int

// проверка, что sizeof(Allocator) + sizeof(Block) + 4 корректно выделит и освободит память под char, а потом выделит под массив int из 9 элементов

// провекра, что sizeof(Allocator) + sizeof(Block) * 4 + 7 корректно выделит для char* c, int* x, char* b, char* d, освободит x и d и корректно выделит для них
// соответсвующие типы еще раз
