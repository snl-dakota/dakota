#!/usr/bin/python

import sys
import re

grab = re.compile('^\[(\d+)\]')

if len(sys.argv) <> 2 :
   print 'Provide exactly one argument'
   exit(1)

filename = sys.argv[1]

procs = set()

try :
  infile = open(filename,'r')
except :
  print 'Error: could not open file',filename
  exit(1)

for line in infile :
  m = grab.match(line)
  if m : 
    procs.add(m.group(1))
infile.close()

print str(len(procs)) + ' processors detected'

procList = []
for p in procs :
   procList.append(p);

procList = sorted(procList)

for p in procList :
  oname = filename + '.' + p
  outfile = open(oname,'w')
  infile = open(filename,'r')
  print "Writing processor " + p
  for line in infile :
    m = grab.match(line)
    if m :
      if m.group(1) == p :
        outfile.write(line)
  outfile.close()
  infile.close()

  
