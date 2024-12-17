#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <semaphore.h>

#define SIZE 1024

int main() {
    int flag = 0;
    int mem= shm_open("/mem", O_RDWR, 0666);
    if (mem == -1) {
        write(1, "shm_open error", sizeof("shm_open error"));
        exit(1);
    }

    char* map = (char*) mmap(0, SIZE, PROT_WRITE, MAP_SHARED, mem, 0);
    if (map == MAP_FAILED) {
        write(1, "mmap error", sizeof("mmap error"));
        exit(1);
    }
    close(mem);

    sem_t* sem2 = sem_open("/sem2", 0); // Семафор для первого и второго дочернего
    if (sem2 == SEM_FAILED) {
        write(1, "sem_open error", sizeof("sem_open error"));
        exit(1);
    }

    sem_t* sem3 = sem_open("/sem3", 0);
    if (sem3 == SEM_FAILED) {
        write(1, "sem_open error", sizeof("sem_open error"));
        exit(1);
    }
    while (1) {
        sem_wait(sem2);

        if (map[0] == 'Q' && map[1] == 'U' && map[2] == 'I' && map[3] == 'T' && map[4] == '\n') {
            sem_post(sem3);
            break;
        }

        for (int i = 0; i < map[SIZE - 1]; i++) {
            if (map[i] == ' ' && flag == 0) {
                flag = 1;
            } else if (map[i] == ' ' && flag == 1) {
                map[i] = 0;
            } else if (map[i] != ' ' && flag == 1) {
                flag = 0;
            }
        }
        flag = 0;
        sem_post(sem3);
    }

    return 0;
}
