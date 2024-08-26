#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define SIZE 10

// Struct per contenere i dati condivisi tra le thread
typedef struct {
    int A[SIZE];
    int B[SIZE];
    int C[SIZE];
    int M; // Media
} shared_data;

// Struct per passare l'indice e il puntatore ai dati condivisi alla thread
typedef struct {
    int index;
    shared_data *data;
} thread_arg;

// Funzione per inizializzare i vettori A e B con valori casuali
void* initialize_vectors(void* arg) {
    shared_data* data = (shared_data*)arg;
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data->A[i] = rand() % 10;
        data->B[i] = rand() % 10;
    }
    pthread_exit(NULL);
}

// Funzione per calcolare l'operazione C[i] = A[i] * B[i]
void* calculate_element(void* arg) {
    thread_arg* t_arg = (thread_arg*)arg;
    int i = t_arg->index;
    shared_data* data = t_arg->data;
    data->C[i] = data->A[i] * data->B[i];
    pthread_exit(NULL);
}

// Funzione per calcolare la media dei valori in C
void* calculate_mean(void* arg) {
    shared_data* data = (shared_data*)arg;
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data->C[i];
    }
    data->M = sum / SIZE;
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[SIZE + 2]; // 1 thread per l'inizializzazione, 10 per le operazioni, 1 per la media
    shared_data data;
    thread_arg t_args[SIZE];

    // Creazione della thread per inizializzare i vettori
    pthread_create(&threads[0], NULL, initialize_vectors, (void*)&data);
    pthread_join(threads[0], NULL); // Attesa del termine della thread di inizializzazione

    // Creazione delle thread per le operazioni C[i] = A[i] * B[i]
    for (int i = 0; i < SIZE; i++) {
        t_args[i].index = i;
        t_args[i].data = &data;
        pthread_create(&threads[i + 1], NULL, calculate_element, (void*)&t_args[i]);
    }

    // Attesa del termine delle thread per le operazioni
    for (int i = 0; i < SIZE; i++) {
        pthread_join(threads[i + 1], NULL);
    }

    // Creazione della thread per calcolare la media
    pthread_create(&threads[SIZE + 1], NULL, calculate_mean, (void*)&data);
    pthread_join(threads[SIZE + 1], NULL); // Attesa del termine della thread della media

    // Stampa della media calcolata
    printf("La media dei valori in C è: %d\n", data.M);

    return 0;
}
