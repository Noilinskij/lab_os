#include "library_second.h"


Allocator* allocator_create(void* const memory, const size_t size) {
    if (memory == NULL || size < sizeof(Allocator) + sizeof(Block) || 
        ((size - sizeof(Allocator) & (size - sizeof(Allocator) - 1)) != 0)) {
        return NULL;
    }
    Allocator* allocator = (Allocator*) memory;
    allocator->memory = memory;
    allocator->size = size;

    Block* first = (Block*) (memory + sizeof(Allocator));
    // память под аллокатор 2^n + sizeof(Allocator)
    first->size = size - sizeof(Allocator) - sizeof(Block);
    first->next = NULL;
    first->status = 0;
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
    //             2^n                                                                            2^(n - 1)
    while ((best->size + sizeof(Block) > size + sizeof(Block)) && (size + sizeof(Block) <= (best->size + sizeof(Block)) / 2)) {
        // новый размер текущего блока
        best->size = (sizeof(Block) + best->size) / 2 - sizeof(Block);
        // блок двойник
        Block* buddy = (Block*) ((uintptr_t) best ^ (best->size + sizeof(Block)));
        buddy->size = best->size;
        buddy->status = 0;
        buddy->next = best->next;
        best->next = buddy;
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
    // объединим свободные блоки
    // если buddy свободен(размер полученного buddy будет равен размеру текущего и status == 0), то объединяем
    // у каждого блока есть buddy, кроме случая когда всего один блок
    Block* buddy = (Block*) ((uintptr_t) current ^ (current->size + sizeof(Block)));
    while(current->size != allocator->size - sizeof(Allocator) - sizeof(Block) &&
          current->size == buddy->size && buddy->status == 0) {
        // если current слева
        if (current->next == buddy) {
            current->next = buddy->next;
            current->size = current->size + sizeof(Block) + buddy->size;
        }
        else {
            buddy->next = current->next;
            buddy->size = buddy->size + sizeof(Block) + current->size;
            current = buddy;
        }
        buddy = (Block*) ((uintptr_t) current ^ (current->size + sizeof(Block)));

    }
    


}


//тесты:
// проверим размер блока ( block->size + sizeof(Block)) для значений памяти(выделяется наименьший доступный)

// проверим, что при выделении 1 байта и его освобождении можем записать максимальный размер для заполнения всей памяти

// проверим, что при выделении памяти для аллокатора память представленная (size - sizeof(Allocator)) является степенью двойки

