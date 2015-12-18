#!/bin/sh

for trace in `ls hamza_format/*`; do
    filename=$(basename "$trace")
    output="${filename%.*}"
    
    ./convert.py  $trace > $output.conv.csv
    
done
