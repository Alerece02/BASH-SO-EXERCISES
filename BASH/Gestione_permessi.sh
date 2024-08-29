#!/bin/bash

# Controlla se è stato passato un argomento (file o directory)
if [ "$#" -ne 1 ]; then
    echo "Uso: $0 <file_o_directory>"
    exit 1
fi

TARGET=$1

# Verifica se il file o la directory esiste
if [ ! -e "$TARGET" ]; then
    echo "Errore: $TARGET non esiste."
    exit 1
fi

# Modifica i permessi del file o della directory
chmod 700 "$TARGET"

# Se il target è una directory, applica i permessi ricorsivamente
if [ -d "$TARGET" ]; then
    find "$TARGET" -type d -exec chmod 700 {} \;
    find "$TARGET" -type f -exec chmod 700 {} \;
fi

# Stampa i nuovi permessi
echo "Nuovi permessi per $TARGET:"
ls -ld "$TARGET"
