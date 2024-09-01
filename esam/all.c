#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>

#define MSG_KEY 1234
#define SEM_KEY 5678
#define SHM_KEY 9876
#define FIFO_PATH "/tmp/myfifo"
#define FILE_PATH "/tmp/myfile.txt"

typedef struct {
    long mtype;
    char message[50];
} message;

// Variabili per pthread
pthread_mutex_t mutex;
pthread_cond_t cond;
int shared_var = 0;

void *thread_func(void *arg) {
    pthread_mutex_lock(&mutex);
    while (shared_var == 0) {
        pthread_cond_wait(&cond, &mutex);
    }
    printf("Thread: Condizione soddisfatta, shared_var = %d\n", shared_var);
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void signal_handler(int sig) {
    printf("Ricevuto segnale %d\n", sig);
}

int main() {
    const char *mess = "ciao";
    message message;

    // ----------------------
    // Creazione e uso della coda di messaggi
    // ----------------------
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    strcpy(message.message, mess);
    message.mtype = 1;
    msgsnd(msgid, &message, sizeof(message.message), 0);
    msgrcv(msgid, &message, sizeof(message.message), 1, 0);
    printf("Messaggio ricevuto dalla coda: %s\n", message.message);
    msgctl(msgid, IPC_RMID, NULL);

    // ----------------------
    // Creazione della memoria condivisa
    // ----------------------
    int shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    int *shared_memory = (int*)shmat(shmid, NULL, 0);
    *shared_memory = 42;
    printf("Valore scritto nella memoria condivisa: %d\n", *shared_memory);
    shmdt(shared_memory);
    shmctl(shmid, IPC_RMID, NULL);

    // ----------------------
    // Creazione e uso della FIFO
    // ----------------------
    mkfifo(FIFO_PATH, 0666);
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    write(fifo_fd, mess, strlen(mess) + 1);
    close(fifo_fd);
    fifo_fd = open(FIFO_PATH, O_RDONLY);
    char buffer[50];
    read(fifo_fd, buffer, sizeof(buffer));
    printf("Messaggio ricevuto dalla FIFO: %s\n", buffer);
    close(fifo_fd);
    unlink(FIFO_PATH);

    // ----------------------
    // Creazione e uso di una pipe anonima
    // ----------------------
    int pipefd[2];
    pipe(pipefd);
    write(pipefd[1], mess, strlen(mess) + 1);
    close(pipefd[1]);
    read(pipefd[0], buffer, sizeof(buffer));
    printf("Messaggio ricevuto dalla pipe: %s\n", buffer);
    close(pipefd[0]);

    // ----------------------
    // Creazione e uso di un semaforo SysV
    // ----------------------
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);
    struct sembuf p = {0, -1, SEM_UNDO};
    struct sembuf v = {0, 1, SEM_UNDO};
    semop(semid, &p, 1);
    printf("Sezione critica\n");
    semop(semid, &v, 1);
    semctl(semid, 0, IPC_RMID);

    // ----------------------
    // Creazione, scrittura, lettura e eliminazione di un file
    // ----------------------
    int file_fd = open(FILE_PATH, O_CREAT | O_WRONLY, 0666);
    write(file_fd, mess, strlen(mess));
    close(file_fd);

    file_fd = open(FILE_PATH, O_RDONLY);
    read(file_fd, buffer, sizeof(buffer));
    printf("Contenuto del file: %s\n", buffer);
    close(file_fd);
    unlink(FILE_PATH);

    // ----------------------
    // Creazione di un processo figlio e attesa della sua terminazione
    // ----------------------
    pid_t pid = fork();
    if (pid == 0) {
        printf("Processo figlio\n");
        exit(0);
    } else {
        wait(NULL);
        printf("Processo figlio terminato\n");
    }

    // ----------------------
    // Gestione dei segnali e delle maschere di segnali
    // ----------------------
    signal(SIGINT, signal_handler);

    // Creazione di una maschera che blocca tutti i segnali tranne SIGINT
    sigset_t set;
    sigfillset(&set); // Aggiunge tutti i segnali alla maschera
    sigdelset(&set, SIGINT); // Rimuove SIGINT dalla maschera
    sigprocmask(SIG_SETMASK, &set, NULL); // Applica la maschera

    printf("Tutti i segnali bloccati tranne SIGINT. Premi Ctrl+C per inviare SIGINT\n");
    sleep(10);

    // Ripristina la maschera dei segnali (nessun segnale bloccato)
    sigemptyset(&set); // Vuota la maschera
    sigprocmask(SIG_SETMASK, &set, NULL); // Applica la maschera vuota (tutti i segnali sbloccati)
    printf("Tutti i segnali sono stati sbloccati.\n");

    // ----------------------
    // Uso di mutex e variabili di condizione con pthreads
    // ----------------------
    pthread_t thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread, NULL, thread_func, NULL);
    sleep(1);
    pthread_mutex_lock(&mutex);
    shared_var = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}