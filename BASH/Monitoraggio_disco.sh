#!/bin/bash

# Soglia di avviso (ad esempio, 90%)
THRESHOLD=90

# Controlla l'occupazione dello spazio su disco per ogni filesystem
df -h | grep -vE '^Filesystem|tmpfs|cdrom' | awk '{ print $5 " " $1 }' | while read output; do
    # Estrai la percentuale di utilizzo e il nome del filesystem
    usep=$(echo $output | awk '{ print $1 }' | sed 's/%//')
    partition=$(echo $output | awk '{ print $2 }')

    # Se l'utilizzo supera la soglia, invia un avviso
    if [ $usep -ge $THRESHOLD ]; then
        echo "Attenzione: lo spazio su disco nella partizione $partition ha raggiunto il $usep%." | mail -s "Avviso spazio su disco" tua_email@example.com
    fi
done
