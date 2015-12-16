#!/usr/bin/python

import sys, re, os

CREATION_TIME_S=1000

READ_RE="^(\d+.\d+);read;(\d+);(\d+);(\d+);(.+)$"
READ_RE2="^(\d+);read;(\d+);(\d+);(\d+);(.+)$"
WRITE_RE="^(\d+.\d+);write;(\d+);(\d+);(\d+);(.+)$"
WRITE_RE2="^(\d+);write;(\d+);(\d+);(\d+);(.+)$"
OPEN_RE="^(\d+.\d+);open;(\d+);(\d+);(\d+);(\d+);(.+)$"
OPEN_RE2="^(\d+);open;(\d+);(\d+);(\d+);(\d+);(.+)$"
CLOSE_RE="^(\d+.\d+);close;(\d+);(.+)$"
CLOSE_RE2="^(\d+);close;(\d+);(.+)$"


current_filenames = []
last_addrs = {}
first_time = -1

def _warn_line(line):
    prog_close2 = re.compile(CLOSE_RE2)
    prog_open2 = re.compile(OPEN_RE2)
    prog_write2 = re.compile(WRITE_RE2)
    prog_read2 = re.compile(READ_RE2)
    
    if prog_read2.match(line):
	print "WARNING READ 2 MATCH"
	return -1
    if prog_write2.match(line):
	print "WARNING WRITE 2 MATCH"
	return -2
    if prog_open2.match(line):
	print "WARNING OPEN 2 MATCH"
	return -3
    if prog_close2.match(line):
	print "WARNING CLOSE 2 MATCH"
	return -4
    return 0
    
def _common_ops(time, name):
    global current_filenames
    global last_addrs
    global first_time
    
    if first_time == -1:
	first_time = time
	
    if name not in current_filenames:
	current_filenames.append(name)
	last_addrs[name] = 0
	
def _common_rw_ops(name, offset, size):
    global last_addrs
    if offset + size > last_addrs[name]:
	last_addrs[name] = offset + size
		
def _conv_time(time):
    global first_time
    if first_time == -1:
	print "ERROR Time"
	sys.exit(-1)
	
    return time - first_time + CREATION_TIME_S
    
if __name__ == "__main__":    
    res = ""
    
    if len(sys.argv) != 2:
	print "Usage : " + sys.argv[0] + " <trace_file>"
	
    prog_read = re.compile(READ_RE)
    prog_write = re.compile(WRITE_RE)
    prog_open = re.compile(OPEN_RE)
    prog_close = re.compile(CLOSE_RE)

    with open(sys.argv[1], "r") as f:
	lines = f.readlines()
    
    for line in lines:
	if prog_read.match(line):
	    groups = prog_read.search(line).groups()
	    time = float(groups[0])
	    fd = int(groups[1])
	    offset = int(groups[2])
	    size = int(groups[3])
	    name = groups[4]
	    
	    _common_ops(time, name)
	    _common_rw_ops(name, offset, size)
		
	    inode_num = current_filenames.index(name)
	    
	    res += str(_conv_time(time)) + ";read;" + str(inode_num) + ";" + str(offset) + ";" + \
		str(size) + "\n"
	    
	# WRITE
	if prog_write.match(line):
	    groups = prog_write.search(line).groups()
	    time = float(groups[0])
	    fd = int(groups[1])
	    offset = int(groups[2])
	    size = int(groups[3])
	    name = groups[4]
	    
	    _common_ops(time, name)
	    _common_rw_ops(name, offset, size)
		
	    inode_num = current_filenames.index(name)
	    
	    res += str(_conv_time(time)) + ";write;" + str(inode_num) + ";" + str(offset) + ";" + \
		str(size) + "\n"
	    
	if prog_open.match(line):
	    groups = prog_open.search(line).groups()
	    time = float(groups[0])
	    name = groups[5]
	    
	    _common_ops(time, name)
		
	    inode_num = current_filenames.index(name)
	    
	    res += str(_conv_time(time)) + ";open;" + str(inode_num) + ";0;0;0" + "\n"
	    
	if prog_close.match(line):
	    groups = prog_close.search(line).groups()
	    time = float(groups[0])
	    name = groups[2]
	    
	    _common_ops(time, name)
		
	    inode_num = current_filenames.index(name)
	    
	    res += str(_conv_time(time)) + ";close;" + str(inode_num) + "\n"
	    
	if _warn_line(line) != 0:
	    sys.exit(-1)
    
    i=0
    for f in last_addrs:
	print str(i) + ".0;open;" + str(current_filenames.index(f)) + ";0;0;0"
	print str(i) + ".1;write;" + str(current_filenames.index(f)) + ";0;" + str(last_addrs[f])
	print str(i) + ".1;close;" + str(current_filenames.index(f))
	i += 1
	
    print res
