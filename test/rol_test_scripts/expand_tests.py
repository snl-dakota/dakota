import os
import re
import subprocess

tests = set()

rootpath = os.getcwd()
workdir = "Extracted_Tests"
if not os.path.isdir(workdir):
  os.makedirs(workdir)

for (dirname, dirs, files) in os.walk('.'):
  if workdir in dirname:
    continue
  for filename in files:
    if filename.endswith('.in') :
      os.chdir(dirname)
      fileandpath = dirname+"/"+filename
      file = open(filename)
      tests.clear()
      for line in file:
        line = line.rstrip()
        #print line
        #vals = re.findall('(#[sp][0-9]+)', line)
        vals = re.findall('#[sp]([0-9]+)', line)
        for val in vals:
          #print val
          tests.add(val)
      print "For "+fileandpath+":"+str(tests)

      #tests.clear()
      command = []
      command.append('')
      for test in tests:
        target_dir = rootpath+'/'+workdir+dirname.lstrip('.')+'/'
        if not os.path.isdir(target_dir):
          os.makedirs(target_dir)
        command[0] = '../dakota_test.perl --extract '+test+' --file-extract="'+target_dir+'test_'+test+'" '+filename
        #print "Executing "+command[0]
        subprocess.call(command, shell=True)
      os.chdir(rootpath)
