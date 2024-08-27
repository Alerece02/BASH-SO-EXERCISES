#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define SHM_KEY 1234
#define ARRAY_SIZE 10

int main(){
    
    int shmid = shmget(SHM_KEY, 3 * ARRAY_SIZE * sizeof(int), 0666);
    if(shmid < 0){
        perror("shmid");
        exit(1);
    }

    int* shared_memory = (int*)shmat(shmid, NULL, 0);
    if(shared_memory == (int*)-1){
        perror("shmat");
        exit(1);
    }


    int* A = shared_memory;
    int* B = shared_memory + ARRAY_SIZE;

    srand(time(NULL));

    for(int i = 0; i < ARRAY_SIZE; i++){
        A[i] = rand() % 10;
        B[i] = rand() % 10;
    }

    if(shmdt(shared_memory) == -1){
        perror("shmdt");
        exit(1);
    }

    return 0;
}