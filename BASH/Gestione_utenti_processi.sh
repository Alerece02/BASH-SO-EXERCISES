#!/bin/bash

if [ "$#" -ne 1 ]; then 
    echo "usage: $0 <nome_utente>"
    exit 1
fi

USER=$1

# veririca se l'utente esiste nel sistema
if ! id -u "$USER" > /dev/null 2>&1; then
    echo "Erroe: utente $USER non esiste."
    exit 1
fi

# visualizza i processi dell'utente
echo "Processi dell'utente $USER:"
# filtra per user e mostra solo le colonne dove è presente il PID E il comando
# che ha avviato il processo
ps -u "$USER" -o pid,comm 

echo "Inserisci il PID del processo da chiudere oppure ALL per chiudere tutti i processi:"
read INPUT

if [ "$INPUT" == "ALL" ]; then
    pkill -u "$USER"
    echo "Tutti i processi dell'utente $USER sono stati terminati."
else
    # verifica che l'input sia un numero intero dove "^" inizio stringa e "$" fine della stringa
    if [[ "$INPUT" =~ ^[0-9]+$ ]]; then
        # uccide forzaramente il processo e se ha successo stampa che è stato terminato
        if kill -9 "$INPUT" > /dev/null 2>&1; then
            echo "Processo di PID $INPUT terminato."
        else
            echo "Errore: impossibile terminare il processo con PID $INPUT."
        fi
    else
        echo "Errore: inout non valido. Inserire un PID valido o ALL."
    fi
fi



