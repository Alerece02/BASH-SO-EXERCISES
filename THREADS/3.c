#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 10
#define NUM_THREADS 4

typedef struct {
    int* array;
    int start;
    int end;
    int* total_sum;
    pthread_mutex_t* mutex_sum;
} ThreadData;

void* sum_array(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_sum = 0;

    for (int i = data->start; i < data->end; i++) {
        local_sum += data->array[i];
    }

    // Proteggi l'aggiornamento della somma totale con un mutex
    pthread_mutex_lock(data->mutex_sum);
    *(data->total_sum) += local_sum;
    pthread_mutex_unlock(data->mutex_sum);

    pthread_exit(NULL);
}

int main() {
    int array[ARRAY_SIZE];
    int total_sum = 0;
    pthread_mutex_t mutex_sum;
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int segment_size = ARRAY_SIZE / NUM_THREADS;

    // Inizializza l'array con valori da 1 a ARRAY_SIZE
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i + 1;
    }

    // Inizializza il mutex
    pthread_mutex_init(&mutex_sum, NULL);

    // Crea i thread
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].array = array;
        thread_data[i].start = i * segment_size;

        // L'ultimo thread deve gestire anche gli elementi rimanenti
        if (i == NUM_THREADS - 1) {
            thread_data[i].end = ARRAY_SIZE;
        } else {
            thread_data[i].end = (i + 1) * segment_size;
        }

        thread_data[i].total_sum = &total_sum;
        thread_data[i].mutex_sum = &mutex_sum;
        pthread_create(&threads[i], NULL, sum_array, (void*)&thread_data[i]);
    }

    // Attendi che tutti i thread finiscano
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Distruggi il mutex
    pthread_mutex_destroy(&mutex_sum);

    printf("La somma totale dell'array è: %d\n", total_sum);

    return 0;
}
