#!/bin/sh

for trace in `ls hamza_format/*`; do
    filename=$(basename "$trace")
    output="${filename%.*}"
    
    ./compute_exec_time.py  $trace > exec_times/$output.csv
    
done
