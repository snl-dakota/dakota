#!/usr/bin/env python

# Verification tool for MMA branch-and-bound
# Jonathan Eckstein, March 2013

# usage : <scriptname> mmaLog1 mmaLog2... [compare run2Log1 run2Log2 ...]

# The first files describe a "master" tree (typically a serial run and
# just one file).  The second list of files are a second run to be
# checked against the first.  Both runs are checked for internal
# consistency.  Then the second is checked against the first, assuming
# the first is correct.  The second tree may have more nodes than the
# first (for example, because it is more parallel and incumbents are
# not being found as quickly relative to the number of nodes explored.


import sys


eCount = 0


def childDescriptor(pDescrip,branchVar,whichChild) :
  return pDescrip[0:branchVar] + \
         str(3 - whichChild) + pDescrip[(branchVar+1):len(pDescrip)]


class mmaNode :
  def __init__(self,descrip_,startBound_,proc_,serial_,parent_):
    self.descrip = descrip_
    self.startBound = startBound_
    self.endBound = -1
    self.proc = proc_
    self.serial = serial_
    self.parent = parent_
    self.children = dict()
    self.branchVar = -1
  def idString(self) :
    return '{' + str(self.proc) + ':' + str(self.serial) + '}'


class mmaTree :

  def __init__(self):
    self.nodes        = dict()
    self.attributes   = -1
    self.observations = -1
    self.fathom       = -1

  def incorporate(self,infile) :
    global eCount
    for line in infile :
      tokens = line.split()
      verb = tokens.pop(0)
      if verb == "attributes" :
        n = int(tokens.pop(0))
        if self.attributes > 0 and self.attributes != n :
          print "Non-matching number of attributes: " \
                + str(self.attributes) + " and " + n
          eCount += 1
        self.attributes = n
      elif verb == "observations" :
        m = int(tokens.pop(0))
        if self.observations > 0 and self.observations != m :
          print "Non-matching number of observations: " \
                + str(self.observations) + " and " + m
          eCount += 1
        self.observations = m
      elif verb == "result" :
        f = float(tokens.pop(0))
        if self.fathom > 0 and self.fathom != f :
          print "Non-matching fathom values: " \
                + str(self.fathom) + " and " + str(f)
          eCount += 1
        self.fathom = f
      elif verb == "create" :
        descrip = tokens.pop(0)
        proc = int(tokens.pop(0))
        serial = int(tokens.pop(0))
        startBound = float(tokens.pop(0))
        parent = tokens.pop(0)
        if not (descrip in self.nodes) :
          node = mmaNode(descrip,startBound,proc,serial,parent)
          self.nodes[descrip] = node
        else :
          node = self.nodes[descrip]
          if node.proc != -1 :
            print "Node created twice : " + node.idString() \
                  + ' {' + str(proc) + ':' + str(serial) + '}'
            eCount += 1
          else :
            node.proc = proc
            node.serial = serial
            node.startBound = startBound
            node.parent = parent
      elif verb == "bound" :
        descrip = tokens.pop(0)
        if not (descrip in self.nodes) :
          node = mmaNode(descrip,-1,-1,-1,-1)
          self.nodes[descrip] = node
        else :
          node = self.nodes[descrip]
        node.endBound = float(tokens.pop(0))
        node.branchVar = int(tokens.pop(0))
        while len(tokens) > 0 :
          whichChild = int(tokens.pop(0))
          startBound = float(tokens.pop(0))
          if whichChild in node.children :
            print "Child " + str(whichChild) + " repeats for " \
                  + node.idString()
            eCount += 1
          else :
            node.children[whichChild] = startBound

  def check(self) :
    global eCount
    for descrip in self.nodes :
      node = self.nodes[descrip]
      if node.endBound > node.startBound :
        print "End bound = " + str(node.endBound) + " > " \
              + str(node.startBound) + " = start bound : " \
              + node.idString()
        eCount += 1
      if node.parent != "root" :
        if not (node.parent in self.nodes) :
          print "Parent does not exist : " + node.idString()
          eCount += 1
        else :
          parent = self.nodes[node.parent]
          if node.startBound > parent.endBound :
            print "Bound mismatch with parent " + node.idString()
            eCount += 1
      for whichChild in node.children :
        cBound = node.children[whichChild]
        cDescrip = childDescriptor(descrip,node.branchVar,whichChild)
        if cDescrip in self.nodes :
          child = self.nodes[cDescrip]
          if child.parent != descrip :
            print "Parent mismatch : " + cDescrip
            eCount +=1
          elif child.startBound != cBound :
            print "Start bound mismatch with parent : " + child.idString()
            eCount += 1
        elif cBound > self.fathom :
          print "Missing child with bound " + str(cBound) + \
                " : " + child.idString()
          eCount += 1

  def checkAgainst(self,master) :
    global eCount
    if self.fathom != master.fathom :
      print "Tree fathom values do not match : " + str(self.fathom) \
            + " and " + str(master.fathom)
      eCount += 1
    if self.attributes != master.attributes :
      print "Attribute counts do not match : " + str(self.attributes) \
            + " and " + str(master.attributes)
      eCount += 1
    for descrip in master.nodes :
      mnode = master.nodes[descrip]
      if descrip in self.nodes :
        node  = self.nodes[descrip]
        if node.startBound != mnode.startBound :
          print "Start bound mismatch: " + str(node.startBound) \
                + ", master=" + str(mnode.startBound) \
                + " : " + node.idString() + " / " + mnode.idString()
          eCount += 1
        if node.endBound != mnode.endBound and node.endBound > self.fathom :
          print "End bound mismatch: " + str(node.endBound) \
                + ", master=" + str(mnode.endBound) \
                + ": " + node.idString() + " / " + mnode.idString()
          eCount += 1
        if node.parent != mnode.parent :
          print "Parent mismatch : " + descrip
          eCount += 1
        if node.branchVar != mnode.branchVar and \
               mnode.endBound > self.fathom :
          # This can happen in some cases, due to some potential
          # children being fathomed and killed earlier in the master
          # tree.  Check whether it looks bad...
          mcbounds = sorted(mnode.children.values())
          cbounds  = sorted(node.children.values())
          while len(cbounds) > 0 :
            b = cbounds.pop()
            if len(mcbounds) > 0 :
              mb = mcbounds.pop()
            else :
              mb = -1
            if b != mb and b > self.fathom :
              print "Branching variable mismatch: " + str(node.branchVar) \
                    + ", master=" + str(mnode.branchVar) \
                    + ": " + node.idString() + " / " + mnode.idString()
              eCount += 1
              break
        else :
          for whichChild in mnode.children :
            cBound = mnode.children[whichChild]
            if cBound > self.fathom :
              if whichChild in node.children :
                cBound2 = node.children[whichChild]
                if cBound2 != cBound :
                  print "Child " + str(whichChild) + " bound mismatch for " \
                        + node.idString() + " / " + mnode.idString()
                  eCount += 1
              elif cBound > self.fathom :
                print "Missing child " + str(whichChild) + " of " + \
                      mnode.idString()
                eCount += 1
      else :  # if not found
        if mnode.startBound > self.fathom and \
           mnode.parent in self.nodes and \
           mnode.parent in master.nodes and \
           self.nodes[mnode.parent].branchVar \
               == master.nodes[mnode.parent].branchVar :
          print "Missing node with bound " + str(mnode.startBound) + \
                " : " + mnode.idString()
          eCount += 1
      

t = mmaTree()

files = sys.argv
files.pop(0)

while len(files) > 0 and files[0] != "compare" :
  filename = files.pop(0)
  try :
    infile = open(filename,'r')
  except :
    print "Error: could not open file",filename
    exit(1)
  print "Reading " + filename
  t.incorporate(infile)

print "Tree has " + str(len(t.nodes)) + " nodes"

t.check()

if files[0] == "compare" :

  print "Reading secondary tree"

  files.pop(0)
  s = mmaTree()

  while len(files) > 0 :
    filename = files.pop(0)
    try :
      infile = open(filename,'r')
    except :
      print "Error: could not open file",filename
      exit(1)
    print "Reading " + filename
    s.incorporate(infile)

  print "Secondary tree has " + str(len(s.nodes)) + " nodes"

  s.check()
  s.checkAgainst(t)

print str(eCount) + " errors"
