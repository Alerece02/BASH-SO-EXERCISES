// client.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>

// Definizione delle chiavi per le IPC
#define MSG_KEY 1234  // Chiave per la message queue
#define SHM_KEY 5678  // Chiave per la memoria condivisa
#define SEM_KEY 9101  // Chiave per il semaforo

// Dimensioni della memoria condivisa
#define SHM_SIZE 1024  // 1 KB
// Nome della FIFO
#define FIFO_NAME "/tmp/my_fifo"  // Nome del file FIFO

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

// Funzione principale del client
void client_main() {
    // Apertura della FIFO (il client si connette alla FIFO creata dal server)
    int fd_fifo = open(FIFO_NAME, O_WRONLY);
    if (fd_fifo == -1) {
        write_error("Errore nell'apertura della FIFO per scrittura\n");
        exit(1);
    }

    // Connessione alla message queue
    int msg_id = msgget(MSG_KEY, 0666);
    if (msg_id == -1) {
        write_error("Errore nella connessione alla message queue\n");
        exit(1);
    }

    // Connessione alla memoria condivisa
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id == -1) {
        write_error("Errore nella connessione alla memoria condivisa\n");
        exit(1);
    }
    char *shm_ptr = shmat(shm_id, NULL, 0); // Attach della memoria condivisa
    if (shm_ptr == (char *)-1) {
        write_error("Errore nell'attach della memoria condivisa\n");
        exit(1);
    }

    // Connessione al semaforo
    int sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        write_error("Errore nella connessione al semaforo\n");
        exit(1);
    }

    request_t req;

    // Invio di una richiesta tramite FIFO
    const char *fifo_msg = "Richiesta da FIFO";
    write(fd_fifo, fifo_msg, strlen(fifo_msg) + 1);
    
    // Invio di una richiesta tramite message queue
    req.type = 1;
    const char *mq_msg = "Richiesta da Message Queue";
    strncpy(req.message, mq_msg, sizeof(req.message) - 1);
    req.message[sizeof(req.message) - 1] = '\0';
    msgsnd(msg_id, &req, sizeof(req.message), 0);

    // Attesa della risposta dal server tramite memoria condivisa
    sem_wait(sem_id); // Blocco del semaforo
    write(STDOUT_FILENO, "Client: Risposta ricevuta dal server: ", 38);
    write(STDOUT_FILENO, shm_ptr, strlen(shm_ptr));
    write(STDOUT_FILENO, "\n", 1);
    sem_signal(sem_id); // Sblocco del semaforo

    // Rimozione delle risorse IPC
    close(fd_fifo);
    shmdt(shm_ptr);
}

int main() {
    client_main(); // Avvia il client
    return 0;
}
