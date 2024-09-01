#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 10

// Struttura dati per passare i parametri alle thread
typedef struct {
    int *A;
    int *B;
    int *C;
    int i;
    float *M;
} thread_data_t;

void *initialize_vectors(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    for (int i = 0; i < SIZE; i++) {
        data->A[i] = rand() % 10;
        data->B[i] = rand() % 10;
    }
    pthread_exit(NULL);
}

void *op(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    data->C[data->i] = data->A[data->i] * data->B[data->i];
    pthread_exit(NULL);
}

void *calculate_mean(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data->C[i];
    }
    *(data->M) = sum / (float)SIZE;
    pthread_exit(NULL);
}

int main() {
    pthread_t init_thread;
    pthread_t op_threads[SIZE];
    pthread_t mean_thread;
    int A[SIZE], B[SIZE], C[SIZE];
    float M;  // Variabile per contenere la media, ora di tipo float
    thread_data_t thread_data;

    srand(time(NULL));

    // Inizializzazione dei vettori A e B
    thread_data.A = A;
    thread_data.B = B;
    pthread_create(&init_thread, NULL, initialize_vectors, (void *)&thread_data);
    pthread_join(init_thread, NULL);

    // Creazione delle thread per l'operazione C[i] = A[i] * B[i]
    for (int i = 0; i < SIZE; i++) {
        thread_data.C = C;
        thread_data.i = i;
        pthread_create(&op_threads[i], NULL, op, (void *)&thread_data);
    }

    // Attendere che tutte le thread di operazione finiscano
    for (int i = 0; i < SIZE; i++) {
        pthread_join(op_threads[i], NULL);
    }

    // Preparazione dei dati per il calcolo della media
    
    thread_data.M = &M;

    // Calcolo della media
    pthread_create(&mean_thread, NULL, calculate_mean, (void *)&thread_data);
    pthread_join(mean_thread, NULL);

    // Stampa della media
    printf("La media dei valori in C è: %.2f\n", M);

    return 0;
}
