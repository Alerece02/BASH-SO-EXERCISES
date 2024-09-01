#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <logfile>"
    exit 1
fi

LOGFILE=$1

# verifica se il file esiste
if [ ! -f "$LOGFILE" ]; then
    echo "Errore: il file $LOGFILE non esiste."
    exit 1
fi

# 1. Trova e stampa tutte le righe contententi indirizzi IP
echo "Indirizzi IP trovati nel file $LOGFILE:"
# trova un numero da 0 a 9 intero che puo essere una sequenza da 1 a 3 cifre, 
# questo pattern deve essere ripetuto 3 volte, alla fine si trova l'ultima sequenza.
# -Eo indica che mi deve prendere solo la parte innerente al patterne non l'intera riga
grep -Eo '([0-9]{1,3}\.){3}[0-9]{1,3}' "$LOGFILE"

REDACTED_FILE="redacted_$LOGFILE"

# sed sostituisce il contenuto di quello che cè dentro alle prime /.../ con quello delle seconde /.../, la g finale 
# dice di tenere in considerazione tutte le righe e non solo la corrente
sed -E 's/([0-9]{1,3}\.){3}[0-9]{1,3}/[REDACTED]/g' "$LOGFILE" > "$REDACTED_FILE"
echo "File redatto salvato come $REDACTED_FILE."

SPECIFIC_IP="192.168.1.1"
# grep -c mi restituisce il numero di righe in un cui appare il pattern specificato
COUNT=$(grep -c "$SPECIFIC_IP" "$LOGFILE")
echo "L'indirizzo IP $SPECIFIC_IP appare $COUNT volte in $LOGFILE"



