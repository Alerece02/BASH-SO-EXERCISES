/*
In questo esercizio, implementerai il classico problema del produttore-consumatore utilizzando 
un buffer circolare (o coda circolare) e i mutex per la sincronizzazione.

Descrizione dell'Esercizio
Hai un buffer circolare condiviso tra un gruppo di thread produttori e un gruppo di thread consumatori. 
I produttori inseriscono elementi nel buffer, mentre i consumatori li prelevano. Dovrai assicurarti che:

I produttori non inseriscano elementi se il buffer è pieno.
I consumatori non prelevino elementi se il buffer è vuoto.
L'accesso al buffer sia sincronizzato in modo che non ci siano condizioni di gara.

Specifiche
Dimensione del buffer: BUFFER_SIZE = 5
Numero di produttori: 3
Numero di consumatori: 3

I produttori generano numeri interi casuali e li inseriscono nel buffer.
I consumatori prelevano numeri dal buffer e li stampano.

*/
/*
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3

typedef struct{
    int *buffer;
    int count;
    int in;
    int out;
    pthread_mutex_t* mutex;
    pthread_cond_t* not_empty;
    pthread_cond_t* not_full;
}BufferData;

int main(){

    int buffer[BUFFER_SIZE] = {0};
    pthread_mutex_t mutex;        
    pthread_cond_t not_empty;      
    pthread_cond_t not_full; 

    pthread_t producers[NUM_PRODUCERS]; // Array per i thread produttori
    pthread_t consumers[NUM_CONSUMERS]; // Array per i thread consumatori
    int producer_ids[NUM_PRODUCERS];    // Array per gli ID dei produttori
    int consumer_ids[NUM_CONSUMERS];    // Array per gli ID dei consumatori

    BufferData buffer_data{
        .buffer = buffer;
        .count = 0;
        .in = 0;
        .out = 0;
        .mutex = &mutex;
        .not_empty = &not_empty;
        .not_full = &not_full;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

 


    return 0;
}
*/