/*
Scrivere un programma C in cui un processo P crea un figlio F. Il figlio F dovr´a eseguire il comando ‘‘cat
/etc/passwd’’ e ridirigere l’output sul file out.txt. Il padre dovr´a creare il file out.txt, creare il figlio F,
attendere la terminazione del figlio, leggere il contenuto del file out.txt e stamparlo a video, usando esclusivamente le system call (System V).
(Si ricorda che per ridirigere l’output ´e utile utilizzare la system call dup, mentre per stampare a video
utilizzando le system call sar´a utile usare il descrittore 1).
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
    pid_t pid;
    int fd;
    
    // Creare il file out.txt con permessi di lettura e scrittura
    fd = creat("out.txt", 0644);
    if (fd < 0) {
        perror("creat");
        exit(1);
    }
    
    // Creare il processo figlio
    pid = fork();
    
    if (pid < 0) {
        // Errore nella fork
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Processo figlio
        
        // Ridirezionare l'output standard verso il file out.txt
        if (dup2(fd, 1) < 0) {
            perror("dup2");
            exit(1);
        }
        
        // Chiudere il file descriptor poiché non è più necessario
        close(fd);
        
        // Eseguire il comando cat /etc/passwd
        execlp("cat", "cat", "/etc/passwd", (char *)NULL);
        
        // Se execlp fallisce
        perror("execlp");
        exit(1);
    } else {
        // Processo padre
        
        // Chiudere il file descriptor poiché non è più necessario
        close(fd);
        
        // Attendere la terminazione del processo figlio
        wait(NULL);
        
        // Riaprire il file out.txt per la lettura
        fd = open("out.txt", O_RDONLY);
        if (fd < 0) {
            perror("open");
            exit(1);
        }
        
        // Leggere il contenuto del file e stamparlo a video
        char buffer[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            if (write(1, buffer, bytes_read) < 0) {
                perror("write");
                exit(1);
            }
        }
        
        // Chiudere il file descriptor
        close(fd);
    }
    
    return 0;
}
