#!/bin/bash

# verifica che sia stato passato solo un argomento (nome del file)
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

# assegna l'argomento a una variabile per il nome del file
FILE=$1

# controlla se il file esite
if [ ! -f "$FILE" ]; then
    echo "Errore: il file $FILE non esiste."
    exit 1
fi

# 1. conta il nimero di righe, parole e caratteri nel file
echo "Contando righe, parole e caratteri nel file $FILE:"
wc "$FILE"

# 2. Trova e stampa tutte le righe contenenti la parola "error"
echo "Ricerca della parola 'error' nel file $FILE:"
grep "error" "$FILE"

# 3. Ordina il file alfabeticamente e salva il risultato in un nuovo file
SORTED_FILE="sorted_$FILE"
echo "Ordinamento del file $FILE e salvataggio in $SORTED_FILE:"
sort "$FILE" > "$SORTED_FILE"

echo "Operazione completata"

