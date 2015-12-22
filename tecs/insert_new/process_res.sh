#!/bin/sh

echo "#db_size;run;real_flash_reads;simu_flash_reads;real_flash_writes;simu_flash_writes;real_jffs2rp;simu_jffs2rp;real_jffs2wb;simu_jffs2wb;real_jffs2we;simu_jffs2we;real_vfsr;simu_vfsr;real_vfsw;simu_vfsw"

for db in "128" "4096"; do 
    for run in `seq 1 5`; do
	# compare flash accesses
	real_flash_reads=`./process_res.py other_traces/flashmon/traces_100_"$db"_$run.csv | grep reads | cut -f 2 -d ":"`
	real_flash_writes=`./process_res.py other_traces/flashmon/traces_100_"$db"_$run.csv | grep writes | cut -f 2 -d ":"`
	simu_flash_reads=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/mtd.txt | grep reads | cut -d ":" -f 2`
	simu_flash_writes=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/mtd.txt | grep writes | cut -d ":" -f 2`
	
	# compare jffs2 accesses
	real_rp_avg=`./process_res.py other_traces/ffs_tracer/ffs_traces_100_"$db"_$run.csv | grep "jffs2_readpage avg" | cut -f 2 -d ":"`
	real_wb_avg=`./process_res.py other_traces/ffs_tracer/ffs_traces_100_"$db"_$run.csv | grep "jffs2_write_begin avg" | cut -f 2 -d ":"`
	real_we_avg=`./process_res.py other_traces/ffs_tracer/ffs_traces_100_"$db"_$run.csv | grep "jffs2_write_end avg" | cut -f 2 -d ":"`
	simu_rp_avg=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/jffs2.txt | grep "jffs2_readpage avg" | cut -f 2 -d ":"`
	simu_wb_avg=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/jffs2.txt | grep "jffs2_write_begin avg" | cut -f 2 -d ":"`
	simu_we_avg=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/jffs2.txt | grep "jffs2_write_end avg" | cut -f 2 -d ":"`
	
	# compare VFS accesses
	real_read_avg=`./process_res.py traces/hamza_format/parsed_100_"$db"_$run.csv | grep "vfs_read avg" | cut -f 2 -d ":"`
	real_write_avg=`./process_res.py traces/hamza_format/parsed_100_"$db"_$run.csv | grep "vfs_write avg" | cut -f 2 -d ":"`
	simu_read_avg=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/vfs.txt  | grep "vfs_read avg" | cut -f 2 -d ":"`
	simu_write_avg=`./process_res.py results/parsed_100_"$db"_$run.conv.csv/vfs.txt  | grep "vfs_write avg" | cut -f 2 -d ":"`
	
	echo "$db;$run;$real_flash_reads;$simu_flash_reads;$real_flash_writes;$simu_flash_writes;$real_rp_avg;$simu_rp_avg;$real_wb_avg;$simu_wb_avg;$real_we_avg;$simu_we_avg;$real_read_avg;$simu_read_avg;$real_write_avg;$simu_write_avg"
	
    done
done
