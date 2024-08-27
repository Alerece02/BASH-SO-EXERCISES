#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

#define TIMEOUT 10
#define MAX_ATTEMPTS 3
#define PASSWORD "password123"  // Password corretta

int pipefd[2];  // File descriptor della pipe

void handle_sigalrm(int sig) {
    write(STDOUT_FILENO, "Timeout! Tempo scaduto.\n", 24);
    exit(1);  // Esce con errore
}

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "Non è possibile terminare con CTRL-C\n", 37);
}

void child_process() {
    char buffer[128];
    
    // Chiude la parte di lettura della pipe nel processo figlio
    close(pipefd[0]);
    
    signal(SIGALRM, handle_sigalrm);  // Gestisce il timeout
    signal(SIGINT, handle_sigint);    // Blocca CTRL-C

    alarm(TIMEOUT);  // Imposta il timer

    write(STDOUT_FILENO, "Inserisci la password: ", 23);
    int len = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (len > 0) {
        buffer[len - 1] = '\0';  // Rimuove il carattere di newline
    }

    alarm(0);  // Ferma il timer

    // Invia la password al padre tramite la pipe
    if (write(pipefd[1], buffer, len) == -1) {
        perror("Errore nella scrittura sulla pipe");
        exit(1);
    }
    close(pipefd[1]);  // Chiude la pipe lato scrittura

    exit(0);  // Termina il processo figlio
}

void parent_process(int attempts) {
    char buffer[128];
    int status;

    while (attempts < MAX_ATTEMPTS) {
        if (pipe(pipefd) == -1) {  // Creazione della pipe per ogni tentativo
            perror("Errore nella creazione della pipe");
            exit(1);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("Errore nella fork");
            exit(1);
        }

        if (pid == 0) {
            // Processo figlio
            close(pipefd[0]); // Chiude la parte di lettura della pipe nel figlio
            child_process();
        } else {
            // Processo padre
            close(pipefd[1]);  // Chiude la parte di scrittura della pipe nel padre
            wait(&status);  // Attende la terminazione del figlio

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                int len = read(pipefd[0], buffer, sizeof(buffer));  // Legge la password dalla pipe
                if (len > 0) {
                    buffer[len] = '\0';  // Aggiunge il terminatore di stringa
                }

                if (strcmp(buffer, PASSWORD) == 0) {
                    write(STDOUT_FILENO, "PWD ESATTA\n", 11);
                    close(pipefd[0]);  // Chiude la pipe lato lettura
                    exit(0);  // Termina il processo padre
                } else {
                    write(STDOUT_FILENO, "PWD ERRATA\n", 11);
                }
            } else {
                write(STDOUT_FILENO, "Tentativo scaduto. Ritenta.\n", 28);
            }

            close(pipefd[0]);  // Chiude la pipe lato lettura dopo ogni tentativo
        }

        attempts++;
    }

    write(STDOUT_FILENO, "Hai superato il numero massimo di tentativi.\n", 47);
    exit(1);  // Termina il processo padre con errore
}

int main() {
    
    signal(SIGINT, handle_sigint);  // Blocca CTRL-C

    parent_process(0);

    return 0;
}
