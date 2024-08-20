#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Uso: $0 <soglia_CPU%> <soglia_MEM_MB>"
    exit 1
fi

soglia_cpu=$1
soglia_mem=$2

log_file="process_killer.log"
exclude_list="systemd|init|bash|sshd|gnome-shell|code|process_killer.sh"  # Escludi processi critici

# Ottieni la memoria totale in MB
total_mem=$(free -m | awk '/^Mem:/ {print $2}')

echo "Monitoraggio processi iniziato..." >> $log_file

while true; do
    ps_output=$(ps -eo pid,%cpu,%mem,comm --sort=-%cpu)
    echo "$ps_output" | while read pid cpu mem comm; do
        if [[ $pid != "PID" ]]; then
            # Calcola la memoria utilizzata in MB
            mem_mb=$(echo "$mem * $total_mem / 100" | bc)
            if (( $(echo "$cpu > $soglia_cpu" | bc -l) )) || (( $(echo "$mem_mb > $soglia_mem" | bc -l) )); then
                if [[ ! "$comm" =~ $exclude_list ]]; then
                    kill -9 $pid
                    echo "[`date +\"%Y-%m-%d %H:%M:%S\"`] Terminato PID $pid ($comm), CPU: $cpu%, MEM: ${mem_mb}MB" >> $log_file
                else
                    echo "Ignorato processo critico: $comm (PID: $pid)" >> $log_file
                fi
            fi
        fi
    done
    sleep 5  # Intervallo aumentato per ridurre il carico
done
