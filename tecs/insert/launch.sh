#!/bin/sh

echo "#trace;simu - number of flash write operations;real - number of flash write operations"

for f in `ls traces/*.csv`; do
	f=`basename $f`
	
	sed "s/__TRACE_FILE__/traces\/$f/" template.cfg > conf.cfg
	./openflash conf.cfg
	
	# flash writes
	simu_flash_write_num=`cat mtd.txt | grep write | wc -l`
	fmon_file=`echo $f | sed -e "s/.conv//" -e "s/parsed/traces/"`
	fmon_file=flashmon/$fmon_file
	real_flash_write_num=`cat $fmon_file | grep W | wc -l`
	
	# flash reads
	simu_flash_read_num=`cat mtd.txt | grep read | wc -l`
	real_flash_read_num=`cat $fmon_file | grep R | wc -l`
	
	echo "$f;$simu_flash_write_num;$real_flash_write_num;$simu_flash_read_num;$real_flash_read_num"
done

# Cleanup
rm jffs2.txt vfs.txt wear_leveling.txt mtd.txt conf.cfg

