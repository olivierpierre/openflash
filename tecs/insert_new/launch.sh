 #!/bin/sh

#~ echo "#trace;simu - number of flash write operations;real - number of flash write operations"

WB_128="5.7"
WB_4096="12.7"
WE_128="30.0"
WE_4096="1000.0"
RP_128="30.0"
RP_4096="240.0"
VFSR_128="30.0"
VFSR_4096="370.0"
VFSW_128="120.0"
VFSW_4096="100.0"

cd traces && ./convert.sh && cd ..

# 128
for f in `ls traces/parsed_100_128_*.csv`; do
	f=`basename $f`

	sed -e "s/__TRACE_FILE__/traces\/$f/" -e "s/__WB_OVERHEAD__/$WB_128/" -e "s/__WE_OVERHEAD__/$WE_128/" -e "s/__RP_OVERHEAD__/$RP_128/" -e "s/__VFS_R_OVERHEAD__/$VFSR_128/"  -e "s/__VFS_W_OVERHEAD__/$VFSW_128/" template.cfg > conf.cfg

	./openflash conf.cfg


	# archive results
	rm -r results/$f/ &> /dev/null
    mkdir results/$f
    cp jffs2.txt mtd.txt wear_leveling.txt vfs.txt conf.cfg results/$f/
done

# 4096
for f in `ls traces/parsed_100_4096_*.csv`; do
	f=`basename $f`

	sed -e "s/__TRACE_FILE__/traces\/$f/" -e "s/__WB_OVERHEAD__/$WB_4096/" -e "s/__WE_OVERHEAD__/$WE_4096/"  -e "s/__RP_OVERHEAD__/$RP_4096/" -e "s/__VFS_R_OVERHEAD__/$VFSR_4096/"  -e "s/__VFS_W_OVERHEAD__/$VFSW_4096/" template.cfg > conf.cfg
	./openflash conf.cfg

	# archive results
	rm -r results/$f/ &> /dev/null
    mkdir results/$f
    cp jffs2.txt mtd.txt wear_leveling.txt vfs.txt results/$f/
done

# Cleanup
rm jffs2.txt vfs.txt wear_leveling.txt mtd.txt conf.cfg

./process_res.sh
