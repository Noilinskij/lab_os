#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <semaphore.h>
#include <signal.h>

#define SIZE 1024

int main() {
    // Открытие разделяемой памяти для получения данных от родителя
    int mem = shm_open("/mem", O_RDWR, 0666);
    if (mem == -1) {
        write(1, "shm_open error", sizeof("shm_open error"));
        exit(1);
    }

    char* map = (char*) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);
    if (map == MAP_FAILED) {
        write(1, "mmap error", sizeof("mmap error"));
        exit(1);
    }

    close(mem);

    sem_t* sem1 = sem_open("/sem1", 0);
    if (sem1 == SEM_FAILED) {
        write(1, "sem_open error", sizeof("sem_open error"));
        shm_unlink("/shmChild1ToChild2");
        exit(1);
    }

    sem_t* sem2 = sem_open("/sem2", O_CREAT, 0666, 0); // Семафор для первого и второго дочернего
    if (sem2 == SEM_FAILED) {
        write(1, "sem_open error", sizeof("sem_open error"));
        sem_unlink("/sem2");
        shm_unlink("/shmChild1ToChild2");
        exit(1);
    }

    kill(atoi(map), SIGUSR1);

    while (1) {
        sem_wait(sem1);
        for (int i = 0; i < map[SIZE - 1]; i++) {
            if ('a' <= map[i] && map[i] <= 'z') {
                map[i] = map[i] + 'A' - 'a';
            }
        }
        if (map[0] == 'Q' && map[1] == 'U' && map[2] == 'I' && map[3] == 'T' && map[4] == '\n') {
            sem_post(sem2);
            break;
        }
        sem_post(sem2);
    }

    sem_unlink("/sem2");
    return 0;
}
