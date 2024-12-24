#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define SIZE 1024


void handler(int sig) {}

void clean() {
    shm_unlink("/mem");
    sem_unlink("/sem1");
    sem_unlink("/sem3");
}


int main() {
    atexit(clean);
    // 0 - стандартный поток ввода
    // 1 - стандартный поток вывода
    signal(SIGUSR1, handler);
    int pid1 = fork();
    if (pid1 > 0) {
        int pid2 = fork(); // создаем здесь, чтобы не появились лишние процессы
        if (pid2 > 0) {

            // Открытие и создание разделяемой памяти 
            int mem = shm_open("/mem", O_RDWR | O_CREAT, 0666);
            if (mem == -1) {
                write(1, "shm_open error", sizeof("shm_open error"));
                exit(1);
            }

            if (ftruncate(mem, SIZE) == -1) {
                write(1, "ftruncate error", sizeof("ftruncate error"));
                exit(1);
            }

            char* map = (char*)mmap(0, SIZE, PROT_WRITE, MAP_SHARED, mem, 0);
            if (map == MAP_FAILED) {
                close(mem);
                write(1, "mmap error", sizeof("mmap error"));
                exit(1);
            }

            close(mem);

            // передаем первому дочернему pid второго, чтобы после отображения и создания семафора, он пробудил второй дочерний
            memset(map, 0, SIZE);
            snprintf(map, SIZE, "%d", pid2);

            sem_t* sem1 = sem_open("/sem1", O_CREAT, 0666, 0); // Семафор для родителя и первого дочернего
            if (sem1 == SEM_FAILED) {
                munmap(map, SIZE);
                write(1, "sem_open error", sizeof("sem_open error"));
                exit(1);
            }

            // Отправляем сигнал первому дочернему процессу
            kill(pid1, SIGUSR1);


            sem_t* sem3 = sem_open("/sem3", O_CREAT, 0666, 0);
            if (sem3 == SEM_FAILED) {
                munmap(map, SIZE);
                write(1, "sem_open error", sizeof("sem_open error"));
                exit(1);
            }

            int n;
            while ((n = read(0, map, SIZE)) != 0) {
                if (map[0] == 'q' && map[1] == 'u' && map[2] == 'i' && map[3] == 't' && map[4] == '\n') {
                    map[SIZE - 1] = n;
                    sem_post(sem1);
                    break;
                }
                map[SIZE - 1] = n;
                sem_post(sem1);
                sem_wait(sem3);
                write(1, map, map[SIZE - 1]);
            }

            wait(NULL);
            wait(NULL);
            munmap(map, SIZE);
        } else if (pid2 == 0) {
            pause();

            if (execl("./child2", "child2", NULL) == -1) {
                write(1, "execl error", sizeof("execl error"));
                exit(1);
            }

        } else {
            write(1, "fork error", sizeof("fork error"));
            exit(1);
        }
    
    } else if (pid1 == 0) {
        pause();
        if (execl("./child1", "child1", NULL) == -1) {
            write(1, "execl error", sizeof("execl error"));
            exit(1);
        }
    } else {
        write(1, "fork error", sizeof("fork error"));
        exit(1);
    }

    return 0;
}
