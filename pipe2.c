#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#define PASSWORD "mypassword"
#define TIMEOUT 10
#define MAX_ATTEMPTS 3

int pipefd[2];

void handle_timeout(int sig) {
    write(STDOUT_FILENO, "\nTempo scaduto!\n", 16);
    close(pipefd[1]); // Chiude la scrittura per il figlio
    exit(1);
}

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "\nNon puoi terminare il programma con CTRL-C!\n", 44);
}

int main() {
    signal(SIGINT, handle_sigint);  // Gestisce il segnale CTRL-C

    int attempts = 0;
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("Errore nella creazione della pipe");
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
                write(pipefd[1], password, strlen(password) + 1);
            } else {
                write(STDOUT_FILENO, "\nErrore nella lettura della password\n", 37);
            }

            close(pipefd[1]);  // Chiude la scrittura per il figlio
            exit(0);
        } else {  // Padre
            wait(NULL);  // Attende che il figlio termini

            char received_password[50];
            close(pipefd[1]);  // Chiude la scrittura per il padre
            read(pipefd[0], received_password, sizeof(received_password));

            if (strcmp(received_password, PASSWORD) == 0) {
                write(STDOUT_FILENO, "PWD ESATTA\n", 11);
                close(pipefd[0]);  // Chiude la lettura per il padre
                exit(0);
            } else {
                write(STDOUT_FILENO, "PWD ERRATA\n", 11);
                attempts++;
            }
        }
    }

    write(STDOUT_FILENO, "Hai superato il numero massimo di tentativi!\n", 46);
    close(pipefd[0]);  // Chiude la lettura per il padre
    return 0;
}
