/*
Secondo il noto algoritmo di Eratostene (240 A.C.), per calcolare i numeri primi compresi tra 2 e 100 `e
sufficiente eliminare tutti i multipli dei numeri 2, 3, 5 e 7 compresi tra 2 e 100. I numeri rimanenti (compresi
2, 3, 5, 7) sono tutti primi.
Scrivere un programma C che utilizzando 4 thread calcoli tutti i numeri primi compresi tra 2 e 100 seguendo
l’algoritmo di Eratostene. La prima thread deve occuparsi di eliminare i multipli di 2, etc. Al termine
dell’elaborazione deve essere visualizzato l’elenco dei numeri primi.
Si ricorda che viene richiesto di utilizzare l’implementazione POSIX per le thread.
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 100

//creazione della struttura per passare i dati ai threads
typedef struct{

    int *numbers;
    int num;
    pthread_mutex_t *mutex;

}thread_data_t;

void *elimina_multipli(void *arg);

int main(){

    pthread_t threads[4];
    thread_data_t thread_data[4];
    pthread_mutex_t mutex;

    //inizializza il mutex
    pthread_mutex_init(&mutex, NULL);

    //inizializza l'array number con valori da 2 a MAX
    int numbers[MAX+1];
    for(int i = 2; i <= MAX; i++){
        numbers[i] = i;
    }

    int numbers_to_check[4] = {2, 3, 5, 7};

    //creazione dei threads per eliminare i multipli
    for(int i = 0; i < 4; i++){
        thread_data[i].numbers = numbers;
        thread_data[i].num = numbers_to_check[i];
        thread_data[i].mutex = &mutex;

        if(pthread_create(&threads[i], NULL, elimina_multipli, &thread_data[i]) != 0){
            perror("pthread_create");
            exit(1);
        }
    }

    // attende che tutti i thread terminino
    for(int i = 0; i < 4; i++){
        if(pthread_join(threads[i], NULL) != 0){
            perror("pthread_join");
            exit(1);
        }
    }

    // Stampa i numeri primi rimasti
    printf("Numeri primi tra 2 e 100:\n");
    for (int i = 2; i <= MAX; i++) {
        if (numbers[i] != 0) {
            printf("%d ", numbers[i]);
        }
    }
    printf("\n");

    pthread_mutex_destroy(&mutex);

    return 0;
}

void *elimina_multipli(void *arg) {
    
    thread_data_t *data = (thread_data_t *)arg;
    int num = data->num;
    int *numbers = data->numbers;

    pthread_mutex_lock(data->mutex);
    for (int i = num * 2; i <= MAX; i += num) {
        numbers[i] = 0;  // Imposta a 0 i multipli del numero
    }
    pthread_mutex_unlock(data->mutex);

    pthread_exit(NULL);

}