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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3

// Struttura per contenere tutte le informazioni necessarie per il buffer
typedef struct {
    int* buffer;                // Puntatore al buffer circolare
    int count;                  // Numero di elementi presenti nel buffer
    int in;                     // Indice di inserimento nel buffer
    int out;                    // Indice di prelievo dal buffer
    pthread_mutex_t* mutex;     // Mutex per proteggere l'accesso al buffer
    pthread_cond_t* not_empty;  // Variabile condizionale per notificare che il buffer non è vuoto
    pthread_cond_t* not_full;   // Variabile condizionale per notificare che il buffer non è pieno
} BufferData;

// Struttura per passare i dati ai thread
typedef struct {
    BufferData* buffer_data;
    int id;  // ID del produttore o consumatore
} ThreadArgs;

void* producer(void* arg) {
    ThreadArgs* thread_args = (ThreadArgs*)arg;
    BufferData* data = thread_args->buffer_data;
    int id = thread_args->id;

    while (1) {
        int item = rand() % 100; // Genera un elemento casuale

        pthread_mutex_lock(data->mutex); // Blocca il mutex per accedere al buffer

        // Attende se il buffer è pieno
        while (data->count == BUFFER_SIZE) {
            pthread_cond_wait(data->not_full, data->mutex); // Attende fino a quando non c'è spazio nel buffer
        }

        // Inserisce l'elemento nel buffer
        data->buffer[data->in] = item;
        data->in = (data->in + 1) % BUFFER_SIZE; // Incrementa l'indice di inserimento in modo circolare
        data->count++; // Incrementa il conteggio degli elementi nel buffer
        printf("Produttore %d ha prodotto: %d\n", id, item);

        // Segnala che il buffer non è più vuoto
        pthread_cond_signal(data->not_empty);

        pthread_mutex_unlock(data->mutex); // Sblocca il mutex

        sleep(1); // Simula il tempo di produzione
    }
    pthread_exit(NULL); // Termina il thread
}

void* consumer(void* arg) {
    ThreadArgs* thread_args = (ThreadArgs*)arg;
    BufferData* data = thread_args->buffer_data;
    int id = thread_args->id;

    while (1) {
        pthread_mutex_lock(data->mutex); // Blocca il mutex per accedere al buffer

        // Attende se il buffer è vuoto
        while (data->count == 0) {
            pthread_cond_wait(data->not_empty, data->mutex); // Attende fino a quando non ci sono elementi nel buffer
        }

        // Preleva l'elemento dal buffer
        int item = data->buffer[data->out];
        data->out = (data->out + 1) % BUFFER_SIZE; // Incrementa l'indice di prelievo in modo circolare
        data->count--; // Decrementa il conteggio degli elementi nel buffer
        printf("Consumatore %d ha consumato: %d\n", id, item);

        // Segnala che il buffer non è più pieno
        pthread_cond_signal(data->not_full);

        pthread_mutex_unlock(data->mutex); // Sblocca il mutex

        sleep(1); // Simula il tempo di consumo
    }
    pthread_exit(NULL); // Termina il thread
}

int main() {
    int buffer[BUFFER_SIZE] = {0}; // Inizializza il buffer circolare a 0
    pthread_mutex_t mutex;         // Mutex per proteggere l'accesso al buffer
    pthread_cond_t not_empty;      // Variabile condizionale per notificare che il buffer non è vuoto
    pthread_cond_t not_full;       // Variabile condizionale per notificare che il buffer non è pieno

    pthread_t producers[NUM_PRODUCERS]; // Array per i thread produttori
    pthread_t consumers[NUM_CONSUMERS]; // Array per i thread consumatori
    ThreadArgs producer_args[NUM_PRODUCERS];  // Argomenti per i produttori
    ThreadArgs consumer_args[NUM_CONSUMERS];  // Argomenti per i consumatori

    // Inizializza la struttura BufferData con i valori iniziali
    BufferData buffer_data = {
        .buffer = buffer,
        .count = 0,
        .in = 0,
        .out = 0,
        .mutex = &mutex,
        .not_empty = &not_empty,
        .not_full = &not_full
    };

    // Inizializza il mutex e le variabili condizionali
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    // Crea i thread produttori
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_args[i].buffer_data = &buffer_data;
        producer_args[i].id = i + 1; // Assegna un ID univoco a ciascun produttore
        pthread_create(&producers[i], NULL, producer, &producer_args[i]); // Crea il thread produttore
    }

    // Crea i thread consumatori
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_args[i].buffer_data = &buffer_data;
        consumer_args[i].id = i + 1; // Assegna un ID univoco a ciascun consumatore
        pthread_create(&consumers[i], NULL, consumer, &consumer_args[i]); // Crea il thread consumatore
    }

    // Unisci i thread (in realtà non raggiungibile perché il ciclo while è infinito)
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL); // Attende che ciascun produttore termini
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL); // Attende che ciascun consumatore termini
    }

    // Distruggi il mutex e le variabili condizionali
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return 0;
}
