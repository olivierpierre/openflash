#!/usr/bin/python

import re, sys

REGEXP="^(\d+.\d+);([a-z]+);.+$"

with open(sys.argv[1], "r") as f:
    lines = f.readlines()
	
    prog=re.compile(REGEXP)
    
    first=True
    last_time = 0.0
    last_name = ""
    for line in lines:
	if prog.match(line):
	    time = float(prog.search(line).groups()[0])
	    name = prog.search(line).groups()[1]
	    
	    if not first:
		print name + ";" + str(time - last_time)
	    first = False
	    last_name = name
	    last_time = time
	    
	    
	
