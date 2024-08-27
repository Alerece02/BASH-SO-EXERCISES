#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#define MSG_KEY 1234

struct msgbuf{
    long mtype;
    char mtext[128];
};

void write_error(const char *msg){
    write(STDERR_FILENO, msg, strlen(msg));
}

void send_message(int msgid, long mtype, const char *message, int delay){
    
    struct msgbuf msg;
    msg.mtype = mtype;
    strncpy(msg.mtext, message, sizeof(msg.mtext) - 1);
    msg.mtext[sizeof(msg.mtext) - 1] = '\0';

    sleep(delay);

    if(msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1){
        write_error("Errore nell'invio del messaggio\n");
        exit(1);
    }

    exit(0);

}

void receive_messages(int msgid){
    struct msgbuf msg;

    for(int i = 1; i <= 3; i++){
        while(msgrcv(msgid, &msg, sizeof(msg.mtext), i, IPC_NOWAIT) != -1){
            printf("Ricevuto messaggio di priorità %ld: %s\n", msg.mtype, msg.mtext);
        }
    }
}

int main(){

    int msgid = msgget(MSG_KEY, 0666 | IPC_CREAT);

    if(msgid == -1){
        write_error("errore creazione coda di messaggi");
        exit(1);
    }

    pid_t pid1 = fork();
    if(pid1 == 0){
        send_message(msgid, 1, "messaggio di alta priorità (processo 1)", 0);
    }

    pid_t pid2 = fork();
    if(pid2 == 0){
        send_message(msgid, 2, "messaggio di alta priorità (processo 2)", 0);
    }


    pid_t pid3 = fork();
    if(pid3 == 0){
        send_message(msgid, 3, "messaggio di alta priorità (processo 3)", 0);
    }


    pid_t pid4 = fork();
    if(pid4 == 0){
        send_message(msgid, 1, "messaggio di alta priorità (processo 4)", 0);
    }


    pid_t pid5 = fork();
    if(pid5 == 0){
        send_message(msgid, 2, "messaggio di alta priorità (processo 5)", 0);
    }

    for(int i = 0; i < 5; i++){
        wait(NULL);
    }

    receive_messages(msgid);

    if(msgctl(msgid, IPC_RMID, NULL) == -1){
        write_error("Errore nella rimozione della coda di messaggi\n");
        exit(1);
    }


    return 0;
}