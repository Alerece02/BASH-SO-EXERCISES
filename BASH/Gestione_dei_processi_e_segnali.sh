#!/bin/bash

handle_signal() { 
    echo "Segnale SIGUSR1 ricevuto dal processo figlio (PID $$)."
}

# fa dormire il processo e lo mette in background
sleep 30 &

# ottine il pid del processo piu recente avviato in background
CHILD_PID=$!

# stampa il pid del processo figlio
echo "Processo figlio avviato con PID $CHILD_PID."

# fa in modo che quando si riceve SIGUSR1 si attivi la funzione handle_signal
trap 'handle_signal' SIGUSR1

# fa dormire il padre per 5 secondi
sleep 5


# invia il segnale SIGUSR1 al processo figlio
kill -SIGUSR1 $CHILD_PID

# attende la terminazione del figlio
wait $CHILD_PID

echo "Processo figlio terminato"