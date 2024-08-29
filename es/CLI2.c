#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_KEY 1234
#define ARRAY_SIZE 10

int main(int argc, char *argv[]){


    int shmid = shmget(SHM_KEY, sizeof(int) * ARRAY_SIZE * 3, 0666);
    if(shmid == -1){
        write(STDERR_FILENO, "Errore SHMGET\n", 20);
        exit(1);
    }

    int *shared_memory = (int*)shmat(shmid, NULL, 0);
    if(shared_memory == (int*)-1){
        write(STDERR_FILENO, "Errore SHMAT\n", 20);
        exit(1);
    }

    int *C = shared_memory + ARRAY_SIZE * 2;
    int *A = shared_memory;
    int *B = shared_memory + ARRAY_SIZE;


    int i = atoi(argv[1]);

    C[i] = A[i] * B[i];

    return 0;
}