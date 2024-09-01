#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TIMEOUT 10
#define MAX_ATTEMPTS 3
#define PASSWORD "password123"  // Password corretta
#define FIFO_NAME "/tmp/password_fifo"  // Nome del file FIFO

void handle_sigalrm(int sig) {
    write(STDOUT_FILENO, "Timeout! Tempo scaduto.\n", 24);
    exit(1);  // Esce con errore
}

void handle_sigint(int sig) {
    write(STDOUT_FILENO, "Non è possibile terminare con CTRL-C\n", 37);
}

void child_process() {
    char buffer[128];
    int fifo_fd;

    signal(SIGALRM, handle_sigalrm);  // Gestisce il timeout
    signal(SIGINT, handle_sigint);    // Blocca CTRL-C

    alarm(TIMEOUT);  // Imposta il timer

    write(STDOUT_FILENO, "Inserisci la password: ", 23);
    int len = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (len > 0) {
        buffer[len - 1] = '\0';  // Rimuove il carattere di newline
    }

    alarm(0);  // Ferma il timer

    // Apre la FIFO per scrittura (modalità bloccante, attende che il padre apra in lettura)
    fifo_fd = open(FIFO_NAME, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Errore nell'apertura della FIFO nel figlio");
        exit(1);
    }

    // Invia la password al padre tramite la FIFO
    if (write(fifo_fd, buffer, len) == -1) {
        perror("Errore nella scrittura sulla FIFO");
        close(fifo_fd);
        exit(1);
    }
    close(fifo_fd);  // Chiude la FIFO

    exit(0);  // Termina il processo figlio
}

void parent_process(int attempts) {
    char buffer[128];
    int fifo_fd;
    int status;

    // Rimuove la FIFO esistente, se presente
    unlink(FIFO_NAME);

    // Crea la FIFO
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("Errore nella creazione della FIFO");
        exit(1);
    }

    while (attempts < MAX_ATTEMPTS) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Errore nella fork");
            exit(1);
        }

        if (pid == 0) {
            // Processo figlio
            child_process();
        } else {
            // Processo padre: apre la FIFO per la lettura
            fifo_fd = open(FIFO_NAME, O_RDONLY);
            if (fifo_fd == -1) {
                perror("Errore nell'apertura della FIFO nel padre");
                exit(1);
            }

            wait(&status);  // Attende la terminazione del figlio

            int len = read(fifo_fd, buffer, sizeof(buffer));  // Legge la password dalla FIFO
            if (len > 0) {
                buffer[len] = '\0';  // Aggiunge il terminatore di stringa
            }
            close(fifo_fd);  // Chiude la FIFO

            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                if (strcmp(buffer, PASSWORD) == 0) {
                    write(STDOUT_FILENO, "PWD ESATTA\n", 11);
                    unlink(FIFO_NAME);  // Rimuove la FIFO
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
    unlink(FIFO_NAME);  // Rimuove la FIFO
    exit(1);  // Termina il processo padre con errore
}

int main() {
    signal(SIGINT, handle_sigint);  // Blocca CTRL-C

    parent_process(0);

    return 0;
}
