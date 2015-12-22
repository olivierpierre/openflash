#!/usr/bin/python

import sys, re
import numpy as np

JFFS2_FIRST_LINE="# JFFS2 Statistics"
STRACE_PARSED_RES_FIRST_LINE="#<time_stamp>;<sys_call>;<arguments;>;<file_name>;<elapsed>"
MTD_FIRST_LINE="# MTD Statistics"
VFS_FIRST_LINE="# VFS Statistics"

FFSTRACER_RE = "^(\d+\.\d+);(\d+\.\d+);(\d+\.\d+);((jffs2_readpage)|(jffs2_write_begin)|(jffs2_write_end));(\d+)$"
FLASHMON_RE = "^\d+\.\d+;(W|R|C|E);.+$"

def _process_vfs_simu_res(path):
    prog = re.compile("^(vfs_read|vfs_write);(\d+|\d+\.\d+);.*$")
    
    with open(path, "r") as f:
	lines = f.readlines()
	
    res = {"vfs_read" : [], "vfs_write" : []}
	
    for line in lines:
	if prog.match(line):
	    name = prog.search(line).groups()[0]
	    time = prog.search(line).groups()[1]
	    res[name].append(float(time))
	    
    read = res["vfs_read"]
    write = res["vfs_write"]
    
    overall_list = read + write
	    
    if len(overall_list) > 0:
	print "overall syscalls:" + str(len(overall_list))
	print "overall avg.:" + str(np.mean(overall_list))
	print "overall stdev.:" + str(np.std(overall_list))
    else:
	print "No syscalls detected ..."
    if len(read) > 0:
	print "vfs_read syscalls:" + str(len(read))
	print "vfs_read avg.:" + str(np.mean(read))
	print "vfs_read stdev.:" + str(np.std(read))
    else:
	print "no vfs_read_detected ..."
    if len(write) > 0:
	print "vfs_write syscalls:" + str(len(write))
	print "vfs_write avg.:" + str(np.mean(write))
	print "vfs_write stdev.:" + str(np.std(write))
    else:
	print "no vfs_write detected ..."
	    
def _process_jffs2_simu_res(path):
    
    prog = re.compile("^(jffs2_write_begin|jffs2_write_end|jffs2_readpage|jffs2_garbage_collect_pass);(\d+\.\d+|\d+).+$")
    
    with open(path, "r") as f:
	lines = f.readlines()
	
    listing = {"jffs2_write_end" : [], "jffs2_write_begin" : [], "jffs2_readpage" : [], "jffs2_garbage_collect_pass" : []}
	
    for line in lines:
	if prog.match(line):
	    name=prog.search(line).groups()[0]
	    time=float(prog.search(line).groups()[1])
	    listing[name].append(time)
	else:
	    if line[0] is not "#":
		print "ERROR line:" + line
		sys.exit(-1)
	   
    rp = listing["jffs2_readpage"]
    wb = listing["jffs2_write_begin"]
    we = listing["jffs2_write_end"]
    overall_list = rp + wb + we    
    
    if len(overall_list) > 0:
	print "overall syscalls:" + str(len(overall_list))
	print "overall avg.:" + str(np.mean(overall_list))
	print "overall stdev.:" + str(np.std(overall_list))
    else:
	print "No syscalls detected ..."
    if len(rp) > 0:
	print "jffs2_readpage syscalls:" + str(len(rp))
	print "jffs2_readpage avg.:" + str(np.mean(rp))
	print "jffs2_readpage stdev.:" + str(np.std(rp))
    else:
	print "no readpage detected ..."
    if len(wb) > 0:
	print "jffs2_write_begin syscalls:" + str(len(wb))
	print "jffs2_write_begin avg.:" + str(np.mean(wb))
	print "jffs2_write_begin stdev.:" + str(np.std(wb))
    else:
	print "no write_begin detected ..."
    if len(we) > 0:
	print "jffs2_write_end syscalls:" + str(len(we))
	print "jffs2_write_end avg.:" + str(np.mean(we))
	print "jffs2_write_end stdev.:" + str(np.std(we))
    else:
	print "no write_end detected ..."
	
def _process_strace(path):
    
    prog_open=re.compile("^(\d+\.\d+);open;\d+;\d+;\d+;\d+;(.*);(\d+\.\d+)$")
    prog_others=re.compile("^\d+\.\d+;((close)|(read)|(write));\d+;.*;(\d+\.\d+)$")
    
    with open(path, "r") as f:
	lines = f.readlines()
	
    listing = {"open" : [], "read" : [], "write" : [], "close" : []}
	
    for line in lines:
	if prog_open.match(line):
	    time = float(prog_open.search(line).groups()[2])*1000000
	    listing["open"].append(time)
	else:
	    if prog_others.match(line):
		name = prog_others.search(line).groups()[0]
		time = float(prog_others.search(line).groups()[4])*1000000
		listing[name].append(time)
	    else:
		if line[0] is not "#":
		    print "ERROR line:" + line
		    sys.exit(-1)
		    
    r = listing["read"]
    w = listing["write"]
    c = listing["close"]
    o = listing["open"]
    overall = r + w + c + o
    
    if len(overall) > 0:
	print "overall syscalls:" + str(len(overall))
	print "overall avg.:%f" % (np.mean(overall))
	print "overall stdev.:%f" % (np.std(overall))
    else:
	print "No syscalls detected ..."
    if len(r) > 0:
	print "vfs_read syscalls:" + str(len(r))
	print "vfs_read avg.:%f" % (np.mean(r))
	print "vfs_read stdev.:%f" % (np.std(r))
    else:
	print "no read detected ..."
    if len(w) > 0:
	print "vfs_write syscalls:" + str(len(w))
	print "vfs_write avg.:%f" % (np.mean(w))
	print "vfs_write stdev.:%f" % (np.std(w))
    else:
	print "no write detected ..."
    if len(o) > 0:
	print "vfs_open syscalls:" + str(len(o))
	print "vfs_open avg.:%f" % (np.mean(o))
	print "vfs_open stdev.:%f" % (np.std(o))
    else:
	print "no open detected ..."
    if len(c) > 0:
	print "vfs_close syscalls:" + str(len(c))
	print "vfs_close avg.:%f" % (np.mean(c))
	print "vfs_close stdev.:%f" % (np.std(c))
    else:
	print "no close detected ..."
	
def _process_mtd_simu_res(path):
    
    prog = re.compile("^(mtd_write|mtd_read|mtd_erase|mtd_read_hit);.+$")
    
    with open(path, "r") as f:
	lines = f.readlines()
	
    res = {"mtd_write" : 0, "mtd_read" : 0, "mtd_erase" : 0, "mtd_read_hit" : 0}
	
    for line in lines:
	if "jffs2_gc" in line:
	    continue
	if prog.match(line):
	    name = prog.search(line).groups()[0]
	    print name
	    res[name] += 1
	else:
	    if line[0] is not '#':
		print "ERROR line:" + line
		sys.exit(-1)
	    
    print "Flash reads:" + str(res["mtd_read"])
    print "Flash writes:" + str(res["mtd_write"])
    print "Flash erases:" + str(res["mtd_erase"])
    print "MTD cache hits:" + str(res["mtd_read_hit"])
	
def _process_ffs_tracer(path):
    
    prog = re.compile(FFSTRACER_RE)
    
    with open(path, "r") as f:
	lines = f.readlines()
	
    listing = {"jffs2_readpage" : [], "jffs2_write_begin" : [], "jffs2_write_end" : []}
	
    for line in lines:
	if prog.match(line):
	    name = prog.search(line).groups()[3]
	    time = float(prog.search(line).groups()[2])*1000000
	    listing[name].append(time)
	else:
	    if line[0] is not "#":
		print "ERROR line:" + line
		sys.exit(-1)
    
    rp = listing["jffs2_readpage"]
    wb = listing["jffs2_write_begin"]
    we = listing["jffs2_write_end"]
    overall_list = rp + wb + we    
    
    if len(overall_list) > 0:
	print "overall syscalls:" + str(len(overall_list))
	print "overall avg.:" + str(np.mean(overall_list))
	print "overall stdev.:" + str(np.std(overall_list))
    else:
	print "No syscalls detected ..."
    if len(rp) > 0:
	print "jffs2_readpage syscalls:" + str(len(rp))
	print "jffs2_readpage avg.:%f" % (np.mean(rp))
	print "jffs2_readpage stdev.:%f" % (np.std(rp))
    else:
	print "no readpage detected ..."
    if len(wb) > 0:
	print "jffs2_write_begin syscalls:" + str(len(wb))
	print "jffs2_write_begin avg.:%f" % (np.mean(wb))
	print "jffs2_write_begin stdev.:%f" % (np.std(wb))
    else:
	print "no write_begin detected ..."
    if len(we) > 0:
	print "jffs2_write_end syscalls:" + str(len(we))
	print "jffs2_write_end avg.:%f" % (np.mean(we))
	print "jffs2_write_end stdev.:%f" % (np.std(we))
    else:
	print "no write_end detected ..."

def _process_flashmon(path):
    prog = re.compile(FLASHMON_RE)
    
    with open(path, "r") as f:
	lines = f.readlines()
	
    res = {"R" : 0, "W" : 0, "E" : 0, "C" : 0}
	
    for line in lines:
	if prog.match(line):
	    name = prog.search(line).groups()[0]
	    res[name] += 1
	else:
	    if line[0] is not "#":
		print "ERROR line:" + line
		sys.exit(-1)
    
    print "Flash reads:" + str(res["R"])
    print "Flash writes:" + str(res["W"])
    print "Flash erases:" + str(res["E"])
    print "Mtd Cache hits:" + str(res["C"])

if __name__ == "__main__":
    
    ffstracer_prog = re.compile(FFSTRACER_RE)
    flashmon_prog = re.compile(FLASHMON_RE)
    
    if len(sys.argv) != 2:
	print "Usage: %s <res file>" % sys.argv[0]
	sys.exit(-1)
    
    path = sys.argv[1]

    with open(sys.argv[1], "r") as f:
	line = f.readline()
	
    if line == JFFS2_FIRST_LINE + "\n":
	print "Detected JFFS2 simulation result file"
	_process_jffs2_simu_res(path)
    else:
	if line == STRACE_PARSED_RES_FIRST_LINE + "\n":
	    print "Detected Strace parsed trace file"
	    _process_strace(path)
	else:
	    if line == MTD_FIRST_LINE + "\n":
		print "Detected MTD simulation result file"
		_process_mtd_simu_res(path)
	    else:
		if ffstracer_prog.match(line):
		    print "Detected fffstracer output file"
		    _process_ffs_tracer(path)
		else:
		    if flashmon_prog.match(line):
			print "Detected flashmon trace"
			_process_flashmon(path)
		    else:
			if line == VFS_FIRST_LINE + "\n":
			    print "Detected JFFS2 simulation result file"
			    _process_vfs_simu_res(path)
			else:
			    print "ERROR: Unknown file ..."
		
	    
