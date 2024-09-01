#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NUM 100
#define NUM_THREADS 4

typedef struct {
    int *numbers;
    int prime;
    pthread_mutex_t *lock;
} ThreadArg;

void initialize_numbers(int *numbers) {
    for (int i = 2; i <= MAX_NUM; i++) {
        numbers[i] = 1;
    }
}

void *sieve(void *arg) {
    ThreadArg *data = (ThreadArg *)arg;
    int prime = data->prime;
    for (int i = 2 * prime; i <= MAX_NUM; i += prime) {
        pthread_mutex_lock(data->lock);
        data->numbers[i] = 0;
        pthread_mutex_unlock(data->lock);
    }
    pthread_exit(NULL);
}

int main() {
    int numbers[MAX_NUM + 1];
    pthread_t threads[NUM_THREADS];
    pthread_mutex_t lock;
    ThreadArg args[NUM_THREADS];
    int primes[NUM_THREADS] = {2, 3, 5, 7};

    initialize_numbers(numbers);

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex init failed\n");
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].numbers = numbers;
        args[i].prime = primes[i];
        args[i].lock = &lock;

        if (pthread_create(&threads[i], NULL, sieve, (void *)&args[i]) != 0) {
            printf("Failed to create thread\n");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);

    printf("Prime numbers between 2 and 100:\n");
    for (int i = 2; i <= MAX_NUM; i++) {
        if (numbers[i]) {
            printf("%d ", i);
        }
    }
    printf("\n");

    return 0;
}
