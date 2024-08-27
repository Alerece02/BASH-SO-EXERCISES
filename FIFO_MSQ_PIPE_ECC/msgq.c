#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define TIMEOUT 10
#define MAX_ATTEMPTS 3
#define PASSWORD "password123"  // Password corretta
#define MSG_KEY 1234  // Chiave per la coda di messaggi

// Struttura del messaggio
struct msgbuf {
    long mtype;
    char mtext[128];
};

void handle_sigalrm(int sig) {
    write(STDOUT_FILENO, "Timeout! Tempo scaduto.\n", 24);
    exit(1);  // Esce con errore
}

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "Non è possibile terminare con CTRL-C\n", 37);
}

void child_process(int msgid) {
    char buffer[128];
    struct msgbuf msg;
    
    signal(SIGALRM, handle_sigalrm);  // Gestisce il timeout
    signal(SIGINT, handle_sigint);    // Blocca CTRL-C

    alarm(TIMEOUT);  // Imposta il timer

    write(STDOUT_FILENO, "Inserisci la password: ", 23);
    int len = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (len > 0) {
        buffer[len - 1] = '\0';  // Rimuove il carattere di newline
    }

    alarm(0);  // Ferma il timer

    // Prepara il messaggio da inviare
    msg.mtype = 1;  // Tipo di messaggio
    strncpy(msg.mtext, buffer, sizeof(msg.mtext) - 1);
    msg.mtext[sizeof(msg.mtext) - 1] = '\0';  // Assicurati che il messaggio sia terminato

    // Invia la password al padre tramite la coda di messaggi
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("Errore nell'invio del messaggio");
        exit(1);
    }

    exit(0);  // Termina il processo figlio
}

void parent_process(int attempts, int msgid) {
    char buffer[128];
    struct msgbuf msg;
    int status;

    while (attempts < MAX_ATTEMPTS) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Errore nella fork");
            exit(1);
        }

        if (pid == 0) {
            // Processo figlio
            child_process(msgid);
        } else {
            // Processo padre
            wait(&status);  // Attende la terminazione del figlio

            // Riceve la password dalla coda di messaggi
            if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
                perror("Errore nella ricezione del messaggio");
                exit(1);
            }

            strncpy(buffer, msg.mtext, sizeof(buffer) - 1);
            buffer[sizeof(buffer) - 1] = '\0';  // Assicurati che il buffer sia terminato

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                if (strcmp(buffer, PASSWORD) == 0) {
                    write(STDOUT_FILENO, "PWD ESATTA\n", 11);
                    // Rimuove la coda di messaggi
                    msgctl(msgid, IPC_RMID, NULL);
                    exit(0);  // Termina il processo padre
                } else {
                    write(STDOUT_FILENO, "PWD ERRATA\n", 11);
                }
            } else {
                write(STDOUT_FILENO, "Tentativo scaduto. Ritenta.\n", 28);
            }
        }

        attempts++;
    }

    write(STDOUT_FILENO, "Hai superato il numero massimo di tentativi.\n", 47);
    // Rimuove la coda di messaggi
    msgctl(msgid, IPC_RMID, NULL);
    exit(1);  // Termina il processo padre con errore
}

int main() {
    signal(SIGINT, handle_sigint);  // Blocca CTRL-C

    // Crea la coda di messaggi
    int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Errore nella creazione della coda di messaggi");
        exit(1);
    }

    parent_process(0, msgid);

    return 0;
}
