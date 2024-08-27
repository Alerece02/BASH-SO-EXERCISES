#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>

#define SHM_KEY 1234
#define ARRAY_SIZE 10

int main(int argc, char *argv[]){

    if(argc != 2){
        write(STDERR_FILENO, "numero di parametri errato\n", 27);
        exit(1);
    }

    int index = atoi(argv[1]);
    if(index < 0 || index >= ARRAY_SIZE){
        write(STDERR_FILENO, "Indice fuori range\n", 19);
        exit(1);
    }

    int shmid;
    int *shared_memory;

    shmid = shmget(SHM_KEY, 3 * sizeof(int) * ARRAY_SIZE, 0666);
        if (shmid == -1) {
        perror("Errore nell'ottenere l'ID della memoria condivisa");
        exit(1);
    }

    shared_memory = (int *)shmat(shmid, NULL, 0);
        if (shared_memory == (void *)-1) {
        perror("Errore nell'attach della memoria condivisa");
        exit(1);
    }

    shared_memory[2 * ARRAY_SIZE + index] = 
    shared_memory[index] * shared_memory[ARRAY_SIZE + index];

    // Deallocazione della memoria condivisa
    if (shmdt(shared_memory) == -1) {
        perror("Errore nel detach della memoria condivisa");
        exit(1);
    }

    return 0;

}