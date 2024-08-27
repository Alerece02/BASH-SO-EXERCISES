# Implementazione di un Sistema Client-Server Multi-Processo con IPC e Thread POSIX

## Obiettivo:
Implementare un sistema client-server che utilizzi diversi meccanismi di comunicazione inter-processo (IPC), tra cui pipe, FIFO, message queue, semafori e memoria condivisa, per gestire la comunicazione tra un client e un server. Il server dovrà essere in grado di gestire più richieste simultanee utilizzando thread POSIX, senza utilizzare variabili globali per condividere dati tra i thread. Tutti gli input e gli output devono essere gestiti utilizzando esclusivamente le system call read() e write().

## Server

## Creazione delle Risorse IPC:
Pipe: Il server deve creare una pipe anonima per ricevere messaggi da un processo figlio.
FIFO: Il server deve creare una FIFO (chiamata /tmp/my_fifo) per ricevere messaggi da client esterni.
Message Queue: Il server deve creare una message queue con una chiave specifica per ricevere richieste dai client.
Memoria Condivisa: Il server deve creare un segmento di memoria condivisa, di dimensione almeno 1024 byte, per inviare le risposte ai client.
Semafori: Il server deve creare un set di semafori per sincronizzare l'accesso alla memoria condivisa tra i thread.

- Gestione delle Richieste:
Thread POSIX: Il server deve gestire le richieste usando thread POSIX. Ogni thread riceverà una richiesta, la elaborerà e scriverà la risposta nella memoria condivisa.
Sincronizzazione: L'accesso alla memoria condivisa deve essere sincronizzato tramite i semafori per evitare condizioni di corsa (race conditions).

- Ricezione e Elaborazione delle Richieste:
Il server deve essere in grado di ricevere richieste da pipe, FIFO e message queue.
Dopo aver ricevuto una richiesta, il server deve creare un thread POSIX che elabori la richiesta e scriva la risposta nella memoria condivisa.

- Gestione delle Risorse:
Il server deve rimuovere tutte le risorse IPC (FIFO, message queue, memoria condivisa, semafori) al termine della sua esecuzione.

## Client

- Connessione alle Risorse IPC:
FIFO: Il client deve aprire la FIFO /tmp/my_fifo e inviare una richiesta al server tramite essa.
Message Queue: Il client deve connettersi alla message queue con la chiave specificata e inviare una richiesta al server.

- Attesa della Risposta:
Memoria Condivisa: Il client deve connettersi al segmento di memoria condivisa e attendere una risposta dal server.
Sincronizzazione: L'accesso alla memoria condivisa deve essere sincronizzato tramite i semafori.

- Output:
Dopo aver ricevuto la risposta dal server tramite la memoria condivisa, il client deve stampare il messaggio ricevuto utilizzando la system call write().

- Gestione delle Risorse:
Il client deve chiudere tutte le connessioni alle risorse IPC utilizzate.
