#! /usr/bin/env python

import os
import sys
import re

if len(sys.argv) == 0:
    print "docking_consensus <input-file>"
    sys.exit(1)

values = []
map = {}

INPUT = open(sys.argv[1],"r")
for line in INPUT:
    line.strip()
    #print line,
    tokens = re.split('[ \t]+', line)
    if len(tokens) > 2 and tokens[1] == "value":
        values.append( eval(tokens[3]) )
    elif len(tokens) > 2 and tokens[0][:8] == "Variable":
        id = eval( tokens[0][9:-1] )
        val = eval( tokens[2] )
        if not id in map:
            map[id] = {}
        if not val in map[id]:
            map[id][val] = 0
        map[id][val] += 1
   
keys = map.keys()
keys.sort()
#for id in keys:
    #for val in map[id].keys():
        #print id,val,map[id][val]

print "Position - (RotamerID Percent)"
for id in keys:
    total=0.0
    for val in map[id].keys():
        total += map[id][val]
    #print "HERE",id,len(values),total,map[id].keys(),map[id]
    if len(values) != total:
        map[id][0] = len(values)-total
    total=1.0*len(values)
    frac = {}
    for val in map[id].keys():
        frac[val] = 100.0*map[id][val] / total

    print str(id)+" - ",
    fkeys = frac.keys()
    fkeys.sort()
    for key in fkeys:
        print "  ("+str(key)+" "+str(frac[key])+")",
    print ""


