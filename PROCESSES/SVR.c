/*
Si scrivano 3 programmi SVR, CLI1 e CLI2 (nel codice indicare con un commento quale dei
tre programmi si sta scrivendo: SVR, CLI1 e CL2).
Il programma SVR, inizializza una o più aree di memoria condivisa in grado di contenere
3 array di interi di 10 elementi cadauno, A, B e C. Genera 1 figlio F1 che esegua il 
programma CLI1 e attende la terminazione del figlio F1.
Il processo padre P, terminato il figlio F1, dovrà generare 10 figli F2.0, F2.1 .. F2.9 
(tra di loro fratelli, che dovranno essere eseguiti potenzialmente in parallelo) che 
eseguiranno tutti CLI2 passando un parametro da 0...9 che indica la posizione ove ogni CLI2 
opera (es"./CLI2 3" opera in posizione 3). Il padre solo dopo la terminazione dei figli F2.0 F2_1 ..., 
calcola la media dei valori nell'array C e la stampa a video. Dopo che il padre termina (o in caso si 
verifichino degli errori nell'esecuzione dei processi figli) deallocare la memoria condivisa. In caso 
di errori, sara' necessario informare l'utente con apposito messaggio e comunque deallocare
la memoria condivisa.
Il programma CLI1, dovrà accedere alla memoria condivisa e popolare gli array A e B con dei valori casuali 
compresi tra 0 e 9 e terminare.
Il programma CLI2, dovrà accedere alla memoria condivisa e calcolare C[i] = A[i] * B[i] dove i deve
essere acquisito come parametro dalla riga di comando (ed "./CLI2 2" calcola C[2] = A[2] * B(2]).
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_KEY 1234
#define ARRAY_SIZE 10


int main(){

    //creaimo lo spazio di memoria
    int shmid = shmget(SHM_KEY, 3 * ARRAY_SIZE * sizeof(int), IPC_CREAT | 0666);
    if(shmid < 0){
        perror("shmget");
        exit(1);
    }

    int* shared_memory = (int*)shmat(shmid, NULL, 0);
    if(shared_memory == (int*)-1){
        perror("shmat");
        exit(1);
    }


    pid_t pid = fork();
    
    if(pid == 0){
        execl("./CLI1", "CLI1", NULL);
        perror("execl");
        exit(1);
    }else if(pid < 0){
        perror("fork");
        exit(1);
    }

    wait(NULL);

    for(int i = 0; i < ARRAY_SIZE; i++){
        pid_t pid_F2 = fork();

        if(pid == 0){
            char param[3];
            itoa(i, param);

            execl("./CLI2", "./CLI2", param, NULL);
            perror("Errore esecuzione CLI2");
            exit(1);
        }
    }

    for(int i = 0; i < ARRAY_SIZE; i++){
        wait(NULL);
    }


    int sum = 0;
    int *C = shared_memory + 2 * ARRAY_SIZE;
    for(int i = 0; i < ARRAY_SIZE; i++){
        sum += C[i];
    }

    float average = sum / (float)ARRAY_SIZE;

    printf("La media dei valori nell'array C è: %.2f\n", average);

    // Deallocazione della memoria condivisa
    if (shmdt(shared_memory) == -1) {
        perror("Errore nel detach della memoria condivisa");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1){
        perror("shmctl");
        exit(1);
    }

    return 0;

}