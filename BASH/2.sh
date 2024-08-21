#!/bin/bash

# Configurazioni generali
cpu_threshold=80
mem_threshold=90
disk_threshold=10
log_file="/var/log/server_monitor.log"
report_file="/var/log/daily_report.log"
auth_log="/var/log/auth.log"
monitor_dir="/var/www/html/uploads"
email="admin@example.com"

# Funzione per monitorare la CPU e la memoria
monitor_resources() {
    echo "$(date): Monitoring resources..." >> $log_file

    # Monitoraggio CPU e memoria, con ordinamento crescente e decrescente
    ps -eo pid,%cpu,%mem,comm --sort=-%cpu | head -n 15 > /tmp/cpu_usage.txt
    ps -eo pid,%cpu,%mem,comm --sort=%cpu | head -n 15 >> /tmp/cpu_usage.txt
    cpu_usage=$(awk '{print $2}' /tmp/cpu_usage.txt | sort -nr | head -n 1)
    mem_usage=$(free | grep Mem | awk '{print $3/$2 * 100.0}')

    if (( $(echo "$cpu_usage > $cpu_threshold" | bc -l) )); then
        echo "$(date): High CPU usage detected: $cpu_usage%" >> $log_file
        awk '$2 > '$cpu_threshold' {print $0}' /tmp/cpu_usage.txt >> $log_file
        awk '$2 > '$cpu_threshold' {print $1}' /tmp/cpu_usage.txt | xargs -I {} renice +10 -p {}
    fi

    if (( $(echo "$mem_usage > $mem_threshold" | bc -l) )); then
        echo "$(date): High memory usage detected: $mem_usage%" >> $log_file
        ps -eo pid,%mem,comm --sort=-%mem | head -n 10 >> $log_file
        ps -eo pid,%mem --sort=-%mem | head -n 5 | awk '{print $1}' | xargs -I {} kill -9 {}
    fi
}

# Funzione per monitorare lo spazio su disco
monitor_disk() {
    echo "$(date): Monitoring disk space..." >> $log_file
    disk_usage=$(df -h | grep '/dev/sda1' | awk '{print $5}' | sed 's/%//')

    if [ "$disk_usage" -gt "$disk_threshold" ]; then
        echo "$(date): Low disk space detected: $disk_usage% used" >> $log_file
        find $monitor_dir -type f -exec du -h {} + | sort -rh > /tmp/disk_usage.txt
        head -n 10 /tmp/disk_usage.txt >> $log_file
        find $monitor_dir -type f -mtime +30 -exec tar -rvf old_files.tar {} \; && gzip old_files.tar
        echo "$(date): Old files archived and deleted from $monitor_dir" >> $log_file
    fi
}

# Funzione per analizzare i log di sicurezza e ordinare IP
analyze_logs() {
    echo "$(date): Analyzing security logs..." >> $log_file
    failed_attempts=$(grep "Failed password" $auth_log | awk '{print $(NF-3)}' | sort | uniq -c | sort -nr)

    echo "$failed_attempts" >> $log_file

    echo "$failed_attempts" | while read count ip; do
        if [ "$count" -gt 5 ]; then
            echo "$(date): Blocking IP $ip due to repeated failed attempts" >> $log_file
            iptables -A INPUT -s $ip -j DROP
        fi
    done

    # Ordinamento degli IP per numero di tentativi
    sort -n <(echo "$failed_attempts") > /tmp/sorted_ips_asc.txt
    sort -nr <(echo "$failed_attempts") > /tmp/sorted_ips_desc.txt
}

# Funzione per generare il report giornaliero avanzato
generate_report() {
    echo "Advanced Daily Server Report - $(date)" > $report_file
    echo "=========================================" >> $report_file
    echo "CPU and Memory Usage (Top 10):" >> $report_file
    cat /tmp/cpu_usage.txt >> $report_file
    echo "" >> $report_file
    echo "Disk Usage (Top 10 largest files):" >> $report_file
    head -n 10 /tmp/disk_usage.txt >> $report_file
    echo "" >> $report_file
    echo "Failed Login Attempts (Sorted):" >> $report_file
    echo "Ascending:" >> $report_file
    cat /tmp/sorted_ips_asc.txt >> $report_file
    echo "Descending:" >> $report_file
    cat /tmp/sorted_ips_desc.txt >> $report_file

    # Invia il report via email
    mail -s "Advanced Daily Server Report" $email < $report_file
}

# Esegui i monitoraggi
monitor_resources
monitor_disk
analyze_logs

# Genera il report giornaliero avanzato (solo se eseguito alle 23:59)
if [ "$(date +%H:%M)" == "23:59" ]; then
    generate_report
fi
