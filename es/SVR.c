#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ARRAY_SIZE 10
#define SHM_KEY 1234

void write_error(const char *message);
void detach_shmemory(int shmid, int *shared_memory);

int main(){

    // CREAZIONE DELLA ZONA DI MEMORIA CONDIVISA
    int shmid = shmget(SHM_KEY, sizeof(int) * ARRAY_SIZE * 3, IPC_CREAT | 0666);
    if(shmid == -1){
        write_error("Errore creazione memoria condivisa\n");
        exit(1);
    }
    
    int *shared_memory = (int*)shmat(shmid, NULL, 0);
    if(shared_memory == (void*)-1){
        write_error("Errore nell'attachment\n");
        exit(1);
    }

    pid_t pid1 = fork();

    if(pid1 == 0){
        execl("./CLI1", "CLI1", NULL);
        write_error("Errore apertura di CLI1\n");
        exit(1);
    }else if(pid1 < 0){
        write_error("Errore nella creazione del processo figlio");
        exit(1);
    }

    wait(NULL);

    for(int i = 0; i < ARRAY_SIZE; i++){
        pid_t pid2 = fork();

        if(pid2 == 0){   
            char buffer[10];
            sprintf(buffer, "%d", i);
            execl("./CLI2", "CLI2", buffer, NULL);
            write_error("Errore apertura di CLI2\n");
            exit(1);
        }else if(pid2 < 0){
            write_error("Errore nella creazione del processo figlio");
            exit(1);
        }
    }

    for(int i = 0; i < ARRAY_SIZE; i++){
        wait(NULL);
    }

    int sum = 0; 
    float average;
    int *C = shared_memory + 2 * ARRAY_SIZE; // Puntatore a C

    for(int i = 0; i < ARRAY_SIZE; i++){
        sum += C[i];
    }

    average = sum / (float) ARRAY_SIZE;

    printf("La media del vettore C è: %.2f\n", average);
    
    if(shmdt(shared_memory) == -1){
        write_error("Errore nel detach\n");
    }

    if(shmctl(shmid, IPC_RMID, NULL) == -1){
        write_error("Errore eliminazione memoria condivisa\n");
    }

    return 0;
}

void write_error(const char *message){
    write(STDERR_FILENO, message, strlen(message));
}
