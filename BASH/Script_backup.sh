#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <directory>"
    exit 1
fi

DIR=$1

# verifica se la directory esiste
if [ ! -d "$DIR" ]; then
    echo "Errore: la directory $DIR non esiste."
    exit 1
fi

BACKUP_DIR="/backup"
if [ ! -d "$BACKUP_DIR" ]; then
    mkdir -p "$BACKUP_DIR"
fi

DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="$BACKUP_DIR/backup_$(basename "$DIR")_$DATE.tar.gz"

tar -czf "$BACKUP_FILE" "$DIR"

if [ -f "$BACKUP_FILE" ]; then
    echo "Backup completato con successo: $BACKUP_FILE"
else
    echo "Errore nella creazione del backup."
fi