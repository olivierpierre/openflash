#!/usr/bin/python

import sys, re, os

READ_RE="^(\d+.\d+);read;(\d+);(\d+);(\d+);(.+)$"
READ_RE2="^(\d+);read;(\d+);(\d+);(\d+);(.+)$"
WRITE_RE="^(\d+.\d+);write;(\d+);(\d+);(\d+);(.+)$"
WRITE_RE2="^(\d+);write;(\d+);(\d+);(\d+);(.+)$"
OPEN_RE="^(\d+.\d+);open;(\d+);(\d+);(\d+);(\d+);(.+)$"
OPEN_RE2="^(\d+);open;(\d+);(\d+);(\d+);(\d+);(.+)$"
CLOSE_RE="^(\d+.\d+);close;(\d+);(.+)$"
CLOSE_RE2="^(\d+);close;(\d+);(.+)$"

if __name__ == "__main__":
    if len(sys.argv) != 2:
	print "Usage : " + sys.argv[0] + " <trace_file>"
	
    prog_read = re.compile(READ_RE)
    prog_read2 = re.compile(READ_RE2)
    prog_write = re.compile(WRITE_RE)
    prog_write2 = re.compile(WRITE_RE2)
    prog_open = re.compile(OPEN_RE)
    prog_open2 = re.compile(OPEN_RE2)
    prog_close = re.compile(CLOSE_RE)
    prog_close2 = re.compile(CLOSE_RE2)

    with open(sys.argv[1], "r") as f:
	lines = f.readlines()
	
    current_filenames = []
    last_addrs = {}
	
    for line in lines:
	if prog_read.match(line):
	    groups = prog_read.search(line).groups()
	    time = groups[0]
	    fd = groups[1]
	    offset = groups[2]
	    size = groups[3]
	    name = groups[4]
	    
	    if name not in current_filenames:
		print "new: " + name
		current_filenames.append(name)
		last_addrs[name] = 0
		
	    if offset + size > last_addrs[name]:
		last_addrs[name] = offset + size
		
	    inode_num = current_filenames.index(name)
	    
	    print time + ";read;" + str(inode_num) + ";" + str(offset) + ";" + \
		str(size)
	    
	    #~ print line.replace(name, str(inode_num))
	    
	# WRITE
	if prog_write.match(line):
	    groups = prog_write.search(line).groups()
	    time = groups[0]
	    fd = groups[1]
	    offset = groups[2]
	    size = groups[3]
	    name = groups[4]
	    
	    if name not in current_filenames:
		print "new: " + name
		current_filenames.append(name)
		last_addrs[name] = 0
		
	    if offset + size > last_addrs[name]:
		last_addrs[name] = offset + size
		
	    inode_num = current_filenames.index(name)
	    
	    print time + ";write;" + str(inode_num) + ";" + str(offset) + ";" + \
		str(size)
	    
	if prog_open.match(line):
	    groups = prog_open.search(line).groups()
	    time = groups[0]
	    name = groups[4]
	    
	    if name not in current_filenames:
		print "new: " + name
		current_filenames.append(name)
		last_addrs[name] = 0
		
	    inode_num = current_filenames.index(name)
	    
	    print time + ";open;" + str(inode_num) + ";0;0;0"
	    
	if prog_close.match(line):
	    groups = prog_close.search(line).groups()
	    time = groups[0]
	    name = groups[2]
	    
	    if name not in current_filenames:
		print "new: " + name
		current_filenames.append(name)
		last_addrs[name] = 0
		
	    inode_num = current_filenames.index(name)
	    
	    print time + ";close;" + str(inode_num)
	    
	if prog_read2.match(line):
	    print "WARNING READ 2 MATCH"
	    os.exit()
	if prog_write2.match(line):
	    print "WARNING WRITE 2 MATCH"
	    os.exit()
	if prog_open2.match(line):
	    print "WARNING OPEN 2 MATCH"
	    os.exit()
	if prog_close2.match(line):
	    print "WARNING CLOSE 2 MATCH"
	    os.exit()
	
    # TODO process last addr
