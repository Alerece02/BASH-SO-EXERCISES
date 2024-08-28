// server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

// Definizione delle chiavi per le IPC
#define MSG_KEY 1234  // Chiave per la message queue
#define SHM_KEY 5678  // Chiave per la memoria condivisa
#define SEM_KEY 9101  // Chiave per il semaforo

// Dimensioni della memoria condivisa
#define SHM_SIZE 1024  // 1 KB
// Nome della FIFO
#define FIFO_NAME "/tmp/my_fifo"  // Nome del file FIFO

// Struttura per passare i dati condivisi tra i thread
typedef struct {
    int sem_id;     // ID del semaforo
    int shm_id;     // ID della memoria condivisa
    char *shm_ptr;  // Puntatore alla memoria condivisa
} shared_data_t;

// Struttura per i messaggi
typedef struct {
    int type;          // Tipo di richiesta (non usato direttamente)
    char message[256]; // Messaggio inviato/ricevuto
} request_t;

// Funzione di utilità per scrivere messaggi di errore su stderr
void write_error(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

// Funzione per eseguire una wait su un semaforo
void sem_wait(int sem_id) {
    struct sembuf sb = {0, -1, 0}; // Setta l'operazione semaforica
    if (semop(sem_id, &sb, 1) == -1) {
        write_error("Errore nella sem_wait\n");
        exit(1);
    }
}

// Funzione per eseguire una signal su un semaforo
void sem_signal(int sem_id) {
    struct sembuf sb = {0, 1, 0}; // Setta l'operazione semaforica
    if (semop(sem_id, &sb, 1) == -1) {
        write_error("Errore nella sem_signal\n");
        exit(1);
    }
}

// Funzione eseguita dai thread per gestire le richieste
void* handle_request(void *arg) {
    shared_data_t *shared = (shared_data_t *)arg;
    
    sem_wait(shared->sem_id); // Blocco del semaforo

    // Simulazione dell'elaborazione della richiesta
    const char *response = "Risposta dal server";
    size_t len = strlen(response);
    memcpy(shared->shm_ptr, response, len); // Copia la risposta nella memoria condivisa
    shared->shm_ptr[len] = '\0'; // Aggiunge il terminatore di stringa

    sem_signal(shared->sem_id); // Sblocco del semaforo

    pthread_exit(NULL); // Termina il thread
}

// Funzione principale del server
void server_main() {
    // Creazione del semaforo
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        write_error("Errore nella creazione del semaforo\n");
        exit(1);
    }
    if (semctl(sem_id, 0, SETVAL, 1) == -1) { // Inizializzazione del semaforo a 1
        write_error("Errore nell'inizializzazione del semaforo\n");
        exit(1);
    }

    // Creazione della memoria condivisa
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        write_error("Errore nella creazione della memoria condivisa\n");
        exit(1);
    }
    char *shm_ptr = shmat(shm_id, NULL, 0); // Attach della memoria condivisa
    if (shm_ptr == (char *)-1) {
        write_error("Errore nell'attach della memoria condivisa\n");
        exit(1);
    }

    shared_data_t shared_data = {sem_id, shm_id, shm_ptr};

    // Creazione della message queue
    int msg_id = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msg_id == -1) {
        write_error("Errore nella creazione della message queue\n");
        exit(1);
    }

    // Creazione della FIFO (il server la crea)
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        write_error("Errore nella creazione della FIFO\n");
        exit(1);
    }
    
    // Apertura della FIFO
    int fd_fifo = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    if (fd_fifo == -1) {
        write_error("Errore nell'apertura della FIFO\n");
        exit(1);
    }

    // Creazione della pipe
    int fd_pipe[2];
    if (pipe(fd_pipe) == -1) {
        write_error("Errore nella creazione della pipe\n");
        exit(1);
    }

    // Creazione di un processo figlio per inviare un messaggio attraverso la pipe
    pid_t pid = fork();
    if (pid == -1) {
        write_error("Errore nella fork\n");
        exit(1);
    }

    if (pid == 0) {
        // Processo figlio - invia un messaggio tramite la pipe
        close(fd_pipe[0]); // Chiude il lato di lettura
        const char *pipe_msg = "Messaggio da Pipe";
        write(fd_pipe[1], pipe_msg, strlen(pipe_msg) + 1);
        close(fd_pipe[1]); // Chiude il lato di scrittura
        exit(0);
    }

    // Server - gestisce le richieste
    while (1) {
        request_t req;
        ssize_t len;

        // Ricezione dei messaggi dalla pipe
        close(fd_pipe[1]); // Chiude il lato di scrittura
        len = read(fd_pipe[0], req.message, sizeof(req.message));
        if (len > 0) {
            req.type = 1;
            write(STDOUT_FILENO, "Server: Ricevuto messaggio dalla Pipe: ", 39);
            write(STDOUT_FILENO, req.message, len);
            write(STDOUT_FILENO, "\n", 1);
        }

        // Ricezione dei messaggi dalla FIFO
        len = read(fd_fifo, req.message, sizeof(req.message));
        if (len > 0) {
            req.type = 2;
            write(STDOUT_FILENO, "Server: Ricevuto messaggio dalla FIFO: ", 39);
            write(STDOUT_FILENO, req.message, len);
            write(STDOUT_FILENO, "\n", 1);
        }

        // Ricezione dei messaggi dalla message queue
        if (msgrcv(msg_id, &req, sizeof(req.message), 0, IPC_NOWAIT) != -1) {
            write(STDOUT_FILENO, "Server: Ricevuto messaggio dalla Message Queue: ", 47);
            write(STDOUT_FILENO, req.message, strlen(req.message));
            write(STDOUT_FILENO, "\n", 1);
        }

        // Creazione di un thread per elaborare la richiesta
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_request, &shared_data) != 0) {
            write_error("Errore nella creazione del thread\n");
        }
        pthread_join(thread, NULL);  // Attende la terminazione del thread

        // Il server continua in un ciclo infinito fino a quando non viene terminato manualmente
    }

    // Rimozione delle risorse IPC
    close(fd_fifo);
    unlink(FIFO_NAME); // Rimuove la FIFO
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    msgctl(msg_id, IPC_RMID, NULL);
}

int main() {
    server_main(); // Avvia il server
    return 0;
}
