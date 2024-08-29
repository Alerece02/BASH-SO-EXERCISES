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
#include <time.h>

#define ARRAY_SIZE 10
#define SHM_KEY 1234

int main(){

    int shmid = shmget(SHM_KEY, ARRAY_SIZE * sizeof(int) * 3, 0666);
    if(shmid == -1){
        write(STDERR_FILENO, "Errore di SHMGET", 18);
        exit(1);
    }

    int *shared_memory = (int*)shmat(shmid, NULL, 0);
    if(shared_memory == (int*) -1){
        write(STDERR_FILENO, "Errore di SHMAT", 18);
        exit(1);
    }

    int *A = shared_memory;
    int *B = shared_memory + ARRAY_SIZE;

    srand(time(NULL));

    for(int i = 0; i < ARRAY_SIZE; i++){
        A[i] = rand() % 10;
        B[i] = rand() % 10;
    }

    return 0;
}

