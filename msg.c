#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define PASSWORD "mypassword"
#define TIMEOUT 10
#define MAX_ATTEMPTS 3

struct msg_buffer {
    long msg_type;
    char msg_text[50];
} message;

void handle_timeout(int sig) {
    write(STDOUT_FILENO, "\nTempo scaduto!\n", 16);
    exit(1);
}

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "\nNon puoi terminare il programma con CTRL-C!\n", 44);
}

int main() {
    signal(SIGINT, handle_sigint);  // Gestisce il segnale CTRL-C

    int attempts = 0;
    pid_t pid;
    key_t key;
    int msgid;

    // Genera una chiave unica per la coda di messaggi
    key = ftok("progfile", 65);

    // Crea una coda di messaggi
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("Errore nella creazione della coda di messaggi");
        exit(1);
    }

    while (attempts < MAX_ATTEMPTS) {
        pid = fork();

        if (pid < 0) {
            perror("Errore nella creazione del processo figlio");
            exit(1);
        }

        if (pid == 0) {  // Figlio
            signal(SIGALRM, handle_timeout);  // Gestisce il segnale di timeout
            alarm(TIMEOUT);  // Imposta il timer

            char password[50];
            write(STDOUT_FILENO, "Inserisci la password: ", 24);
            ssize_t n = read(STDIN_FILENO, password, sizeof(password) - 1);

            alarm(0);  // Disabilita il timer se la password è stata inserita
            if (n > 0) {
                password[n - 1] = '\0';  // Rimuove il carattere di newline
                message.msg_type = 1;
                strcpy(message.msg_text, password);
                msgsnd(msgid, &message, sizeof(message), 0);
            } else {
                write(STDOUT_FILENO, "\nErrore nella lettura della password\n", 37);
            }

            exit(0);
        } else {  // Padre
            wait(NULL);  // Attende che il figlio termini

            msgrcv(msgid, &message, sizeof(message), 1, 0);

            if (strcmp(message.msg_text, PASSWORD) == 0) {
                write(STDOUT_FILENO, "PWD ESATTA\n", 11);
                msgctl(msgid, IPC_RMID, NULL);  // Rimuove la coda di messaggi
                exit(0);
            } else {
                write(STDOUT_FILENO, "PWD ERRATA\n", 11);
                attempts++;
            }
        }
    }

    write(STDOUT_FILENO, "Hai superato il numero massimo di tentativi!\n", 46);
    msgctl(msgid, IPC_RMID, NULL);  // Rimuove la coda di messaggi
    return 0;
}
