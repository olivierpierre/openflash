#!/bin/sh

for trace in `ls hamza_format/*`; do
    filename=$(basename "$trace")
    output="${filename%.*}"
    
    sed -e "s/\/mnt\/flash\/test.db$/0/" -e "s/\/mnt\/flash\/test.db-journal$/1/" -e "s/\/mnt\/flash$/2/" $trace > $output.conv.csv
    
done
