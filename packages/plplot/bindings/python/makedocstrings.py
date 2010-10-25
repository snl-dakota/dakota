# Copyright 2002 Gary Bishop
# This file is part of PLplot.

# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; version 2 of the License.

# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.

# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

# The recent versions of SWIG (at least for 1.3.11) do not handle documentation
# strings correctly
# This script is a quick hack to fixup the doc strings in the 
# SWIG-generated c code, but presumably this script
# will not always be necessary

import sys, re

def main():
  if len(sys.argv) != 3:
    print 'usage: makedocstrings infile outfile'
    
  infile = open(sys.argv[1], 'rt')
  outfile = open(sys.argv[2], 'wt')

  docstrings = {}
  
  while 1:
    line = infile.readline()
    if not line:
      break
    m = re.match(r'#define _doc_([a-zA-Z_0-9]+)', line)
    if m:
      name = m.group(1)
      value = '_doc_'+name
      docstrings[name] = value
      #print 'got',name

    if re.match(r'static PyMethodDef SwigMethods', line):
      outfile.write(line)
      #print 'here'
      while 1:
        line = infile.readline()
        m = re.match('[ \t]+\{[ \t]\(char \*\)"([a-zA-Z_0-9]+)"(.*)\, NULL \},', line)
        if not m:
          m = re.match('[ \t]+\{[ \t]\(char \*\)"([a-zA-Z_0-9]+)"(.*)\},', line)
          if not m:
            break
        func = m.group(1)
        #print 'look for',func
        if func in docstrings.keys():
          line = '\t{ (char *)"%s"%s, %s },\n' % (func, m.group(2), docstrings[func])
        outfile.write(line)

    outfile.write(line)

if __name__ == '__main__':
  main()
