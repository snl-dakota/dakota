#!/usr/bin/env python

# Visualization tool for PEBBL load logs
# Jonathan Eckstein, March 2010

# To-do:
#  Black-and-white output


import sys
from optparse import OptionParser
import re
import string

warningIssued = False


def extractToken(string,wantWarning) :
  global warningIssued
  try :
    m = re.match('(\S+)\s*(.*)$',string)
    return (m.group(1),m.group(2))
  except :
    if wantWarning :
       if not warningIssued :
         print "Warning: missing data -- log file may be old, shorter format"
         warningIssued = True
       return("0","")
    else :
      print "Error:  trouble parsing input file",filename
      print "   Make sure all input files were generated",
      print "through the PEBBL"
      print "   --loadLogSeconds=t option."
      print "Error encountered in line",records+1,":"
      print "  ",line
      exit(1)

def getInt(string,wantWarning,target) :
  (token,string) = extractToken(string,wantWarning)
  target.append(int(token))
  return string

def getFloat(string,wantWarning,target) :
  (token,string) = extractToken(string,wantWarning)
  target.append(float(token))
  return string

def plural(value) :
  if value == 1 :
    return ""
  return "s"

def procset(olist,procs,workers,hubs,default) :
  if olist == None :
    return default
  ps = set()
  for item in olist :
    litem = string.lower(item)
    matchobj = re.match('^(\d+)\-(\d+)$',litem)
    if matchobj <> None :
      brange = int(matchobj.group(1))
      erange = int(matchobj.group(2))
      if erange >= brange :
        ps |= set(range(brange,erange+1))
      else :
        print "Warning -- invalid range " + str(brange) + "-" + str(erange) \
              + " ignored"
    elif item.isdigit() :
      ps.add(int(item))
    elif litem == "all" :
      ps = procs
    elif litem == "hubs" or litem=="hub" :
      ps |= hubs
    elif litem == "worker" or litem=="workers" :
      ps |= workers
    elif litem == "root" :
      ps.add(min(procs))
    elif litem == "none" :
      ps = set()
    else :
      print "Error: unrecognized processor specification: " + item
      exit(1)
  ps &= procs
  return ps
    

parser = OptionParser(usage="%prog [options] loadLog1 [loadLog2 ...]",
                      version="%prog 1.1")

parser.add_option("", "--workers",dest="wantWorkers",action="store_true",
                  default=True,help="Graph worker loads (default)")
parser.add_option("", "--noworkers",dest="wantWorkers",action="store_false",
                  help="Do not graph worker loads")

parser.add_option("", "--hubs",dest="wantHubs",action="store_true",
                  default=True,help="Graph hub loads (default)")
parser.add_option("", "--nohubs",dest="wantHubs",action="store_false",
                  help="Do not graph hub loads")

parser.add_option("", "--boundops",dest="wantBoundOps",action="store_true",
                  default=False,help="Graph bounding operations")
parser.add_option("", "--noboundops",dest="wantBoundOps",action="store_false",
                  help="Do not graph bounding operations (default)")

parser.add_option("", "--boundrate",dest="wantBoundRate",action="store_true",
                  default=False,help="Graph bounding operations per second")
parser.add_option("", "--noboundrate",dest="wantBoundRate",action="store_false",
                  help="Do not graph bounding operations per second (default)")

parser.add_option("", "--gloads",dest="wantGLoads",action="store_true",
                  default=False,help="Graph global load estimates")
parser.add_option("", "--nogloads",dest="wantGLoads",action="store_false",
                  help="Do not graph global load estimates (default)")

parser.add_option("", "--cloads",dest="wantCLoads",action="store_true",
                  default=False,help="Graph cluster load estimates")
parser.add_option("", "--nocloads",dest="wantCLoads",action="store_false",
                  help="Do not graph cluster load estimates (default)")

parser.add_option("", "--releases",dest="wantReleases",action="store_true",
                  default=False,help="Graph subproblem releases")
parser.add_option("", "--noreleases",dest="wantReleases",action="store_false",
                  help="Do not graph subproblem releases (default)")

parser.add_option("", "--solgen",dest="wantSolGen",action="store_true",
                  default=False,help="Graph solutions generated")
parser.add_option("", "--nosolgen",dest="wantSolGen",action="store_false",
                  help="Do not graph solution generation (default)")

parser.add_option("", "--admit",dest="wantAdmit",action="store_true",
                  default=False,help="Graph solutions admitted to repository")
parser.add_option("", "--noadmit",dest="wantAdmit",action="store_false",
                  help="Do not graph solution repository admissions (default)")

parser.add_option("-p","--proc",dest="plist",action="append",
                  type="string",metavar="P",
                  help="Graph data for processor P; may be given multiple "
                  "times; specific procssor ranks, 'hubs', 'workers', 'all'"
                  " (default) allowed; "
                  "inclusive ranges of processors indicated with P-Q")

parser.add_option("-b","--bounds",dest="blist",action="append",
                  type="string",metavar="P",
                  help="Graph bounds for processor P; may be given multiple "
                  "times; same syntax as --proc")
parser.add_option("-i","--incumbents",dest="ilist",action="append",
                  type="string",metavar="P",
                  help="Graph incumbents for processor P; can specify multiple"
                  " times; same syntax as --proc")
parser.add_option("","--gbounds",dest="glist",action="append",
                  type="string",metavar="P",
                  help="Graph global bounds at processor P; can specify "
                  "multiple times; same syntax as --proc")
parser.add_option("","--cbounds",dest="clist",action="append",
                  type="string",metavar="P",
                  help="Graph cluster bounds at processor P; can specify "
                  "multiple times; same syntax as --proc")
parser.add_option("","--hbounds",dest="hlist",action="append",
                  type="string",metavar="P",
                  help="Graph hub pool bounds at processor P; can specify "
                  "multiple times; same syntax as --proc")
parser.add_option("","--sbounds",dest="slist",action="append",
                  type="string",metavar="P",
                  help="Graph server pool bounds at processor P; can specify "
                  "multiple times; same syntax as --proc")

parser.add_option("","--dispatches",dest="wantDispatch",action="store_true",
                  default=False,help="Graph subproblems dispatched from hubs")
parser.add_option("", "--nodispatches",dest="wantDispatch",action="store_false",
                  help="Do not graph subproblem hub dispatches (default)")
parser.add_option("","--receives",dest="wantReceive",action="store_true",
                  default=False,help="Graph subproblems dispatched from hubs")
parser.add_option("", "--noreceives",dest="wantReceive",action="store_false",
                  help="Do not graph subproblem hub dispatches (default)")
parser.add_option("","--rebals",dest="wantRebal",action="store_true",
                  default=False,help="Graph subproblem rebalances to hubs")
parser.add_option("", "--norebals",dest="wantRebal",action="store_false",
                  help="Do not graph subproblem rebalances (default)")
parser.add_option("","--lbrounds",dest="wantLB",action="store_true",
                  default=False,help="Graph number of load balancing rounds")
parser.add_option("", "--nolbrounds",dest="wantLB",action="store_false",
                  help="Do not graph load balancing rounds (default)")

parser.add_option("", "--startseconds",dest="startseconds",default="0",
                  type="float",metavar="S1",
                  help="End of chart time window (default 0)")
parser.add_option("", "--endseconds",dest="endseconds",default="0",
                  type="float",metavar="S2",
                  help="End of chart time window (default end of run)")

parser.add_option("", "--show",dest="wantScreen",action="store_true",
                  default=False,help="Display graph on screen")
parser.add_option("", "--noshow",dest="wantScreen",action="store_false",
                  help="Do not show graph on screen (default)")

parser.add_option("", "--format",dest="outputType",default="pdf",
                  metavar="FORMAT",
                  help="Write output using FORMAT  [default: "
                  "%default, must be recognized by matplotlib/TkAgg]")

parser.add_option("", "--nooutput",dest="wantOutput",action="store_false",
                  default=True,help="Suppress output file")
parser.add_option("","--suffix",dest="suffix",default="",metavar="SUFFIX",
                  help="Output file name is problemNameSUFFIX.FORMAT")
parser.add_option("","--pname",dest="pname",metavar="PNAME",
                  help="Override problem name derived from input file")

parser.add_option("","--launch",dest="launch",action="store_true",
                  default=False,
                  help="Immediately launch separate viewer process")
parser.add_option("","--viewer",dest="viewer",default="xpdf",
                  metavar="VIEWER",
                  help="Viewer program to launch (default: %default)")

parser.add_option("", "--legend",dest="wantLegend",action="store_true",
                  default=True,help="Display legend (default)")
parser.add_option("", "--nolegend",dest="wantLegend",action="store_false",
                  help="Do not display legend")


(options, args) = parser.parse_args()

if len(args) == 0 :
  print "Warning: no loadLog file arguments specified, so no action taken."
  exit()

if options.startseconds < 0 or options.endseconds < 0 :
  print "Error: negative time value specified."
  exit(1)

if options.endseconds <> 0 and options.startseconds > options.endseconds :
  print "Error: startseconds is larger than endseconds."
  exit(1)

try:
   import matplotlib as mpl
   if not options.wantScreen :
     mpl.use('Agg')
   import matplotlib.pyplot as pl
except ImportError:
   print "Matplotlib is not available in this python installation"
   print "If running from acro-xxx/python/bin or using either lpython"
   print "  or lbin, try the following to make matplotlib available"
   print "  to acro's virtual python installation:"
   print "     Install scipy in your regular python system"
   print "     In acro-xxx/"
   print "        rm -rf python"
   print "        svn.a update"
   print "        ./setup --site-packages"
   print "If running from your regular python system, install scipy there."
   sys.exit(1)

colors = ["black","red","blue","green","magenta","gray","orange","purple", 
          "brown","darkcyan","cadetblue","coral","crimson","chocolate","cyan",
          "goldenrod","darkslateblue","lawngreen","lightseagreen",
          "mediumvioletred","midnightblue","olive","rosybrown","royalblue",
          "seagreen","slateblue","tomato","yellow","sandybrown","plum",
          "yellowgreen","wheat","violet","turquoise","thistle","teal",
          "springgreen","slategray","slateblue","skyblue","sienna","salmon",
          "powderblue","peru","palevioletred","orangered","olivedrab",
          "orchid","navy","mediumturquoise","mediumseagreen",
          "mediumaquamarine","maroon","lawngreen","indigo","indianred",
          "forestgreen","gold","firebrick","dodgerblue",
          "deeppink","deepskyblue","darkviolet","darkslategray",
          "darkslateblue","darkmagenta","darkgreen","darkgoldenrod",
          "blueviolet","aquamarine"]

ncolors = len(colors)

for filename in args :

  try :
    infile = open(filename,'r')
  except :
    print "Error: could not open file",filename
    exit(1)

  if options.pname == None :
    if filename.endswith(".loadLog") :
      problemname = filename[0:len(filename)-1-len("loadLog")]
    else :
      problemname = filename
  else :
    problemname = options.pname

  print 'Filename %s, problem name %s:' % (filename,problemname)

  processor = []
  time = []
  bcalls = []
  sps = []
  hubsps = []
  totalsps = []
  serversps = []
  gloadest = []
  cloadest = []
  release = []
  bounds = []
  incumbents = []
  gbounds = []
  cbounds = []
  hbounds = []
  sbounds = []
  offers  = []
  admits  = []
  dispatches = []
  receives = []
  rebals = []
  lbrounds = []

  records = 0

  for line in infile:
    line = getInt(line,False,processor)
    line = getFloat(line,False,time)
    line = getInt(line,False,bcalls)
    line = getInt(line,False,sps)
    line = getInt(line,False,hubsps)
    line = getInt(line,False,totalsps)
    line = getInt(line,False,serversps)
    line = getInt(line,True,gloadest)
    line = getInt(line,True,cloadest)
    line = getInt(line,True,release)
    line = getFloat(line,True,bounds)
    line = getFloat(line,True,incumbents)
    line = getFloat(line,True,gbounds)
    line = getFloat(line,True,cbounds)
    line = getFloat(line,True,hbounds)
    line = getFloat(line,True,sbounds)
    line = getInt(line,True,offers)
    line = getInt(line,True,admits)
    line = getInt(line,True,dispatches)
    line = getInt(line,True,receives)
    line = getInt(line,True,rebals)
    line = getInt(line,True,lbrounds)
    records = records + 1

  procs = set(processor)
  nprocs = len(procs)

  hubs = set()
  workers = set()
  bprocs = set()
  iprocs = set()
  gprocs = set()
  cprocs = set()
  hprocs = set()
  sprocs = set()

  for i in range(records):
    if sps[i] > 0 or release[i] > 0: 
      workers.add(processor[i])
    if hubsps[i] > 0 :
      hubs.add(processor[i])
    if abs(bounds[i]) < 1e75 :
      bprocs.add(processor[i])
    if abs(incumbents[i]) < 1e75 :
      iprocs.add(processor[i])
    if abs(gbounds[i]) < 1e75 :
      gprocs.add(processor[i])
    if abs(cbounds[i]) < 1e75 :
      cprocs.add(processor[i])
    if abs(hbounds[i]) < 1e75 :
      hprocs.add(processor[i])
    if abs(sbounds[i]) < 1e75 :
      sprocs.add(processor[i])
      
  nworkers = len(workers)
  nhubs = len(hubs)

  print '   %d observation%s, %d processor%s, %d worker%s, %d hub%s' % \
        (records,plural(records),nprocs,plural(nprocs),
         nworkers,plural(nworkers),nhubs,plural(nhubs))
  sumbcalls = sum(bcalls)
  print '   %d subproblem%s bounded' % (sumbcalls,plural(sumbcalls))

  if records == 0 :
     print "   Warning: no data in",filename,"-- skipping graph generation."
     continue

  pset    = procset(options.plist,procs,workers,hubs,procs)
  procs   = procs & pset
  hubs    = hubs & pset
  workers = workers & pset

  tmax = max(time)
  startseconds = options.startseconds
  if startseconds > tmax :
    print "   Warning: startseconds after end of run.  No output produced."
    exit(1)
  endseconds   = options.endseconds
  if endseconds == 0 or endseconds > tmax:
     endseconds = tmax

  spPlotted  = False
  valPlotted = False

  peakLoad = -1
  peakProc = -1
  peakTime = -1
  peakType = "nothing"

  fig = pl.figure()
  ax1 = fig.add_subplot(111)

  ymax = 0

  if options.wantWorkers :
    for p in workers :
      x = []
      y = []
      for i in range(records):
        if processor[i]==p and time[i]>=startseconds and time[i]<=endseconds:
           x.append(time[i])
           y.append(sps[i])
           if sps[i] > peakLoad :
             peakLoad = sps[i]
             peakProc = processor[i]
             peakTime = time[i]
             peakType = "worker"
           if sps[i] > ymax :
             ymax = sps[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'-',color=colors[p % ncolors],
                 label="Worker " + str(p) + " pool")

  if options.wantHubs :
    for p in hubs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds and time[i]<=endseconds:
           x.append(time[i])
           y.append(hubsps[i])
           if hubsps[i] > peakLoad :
             peakLoad = hubsps[i]
             peakProc = processor[i]
             peakTime = time[i]
             peakType = "hub"
           if hubsps[i] > ymax :
             ymax = hubsps[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'--',color=colors[p % ncolors],
                label="Hub " + str(p) + " pool")

  if peakType != "nothing" :
    print "   Peak load is " + str(peakLoad) + " subproblems at " \
          + peakType + " " + str(peakProc) + " at time " + str(peakTime)

  if options.wantBoundOps :
    for p in workers :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and bcalls[i] > 0:
           x.append(time[i])
           y.append(bcalls[i])
           if bcalls[i] > ymax :
             ymax = bcalls[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'.',color=colors[p % ncolors],markersize=2, 
                 label="Worker " + str(p) + " bound ops")

  if options.wantBoundRate :
    for p in workers :
      x = []
      y = []
      lastTime = -1
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds :
          if lastTime > -1 :
            x.append(time[i])
            temp = bcalls[i]/(time[i] - lastTime)
            y.append(temp)
            if temp > ymax :
              ymax = temp
          lastTime = time[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'o',color=colors[p % ncolors],markersize=4, 
                 label="Worker " + str(p) + " bounds/second")

  if options.wantGLoads :
    for p in procs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds and time[i]<=endseconds:
           x.append(time[i])
           y.append(gloadest[i])
           if gloadest[i] > ymax :
             ymax = gloadest[i]
      if len(x) > 0 :
        spPlotted = True;
        pl.plot(x,y,'-+',color=colors[p % ncolors],
                markersize=6,
                label="Global load est at " + str(p))

  if options.wantCLoads :
    for p in procs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds and time[i]<=endseconds:
           x.append(time[i])
           y.append(cloadest[i])
           if cloadest[i] > ymax :
             ymax = cloadest[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'-*',color=colors[p % ncolors],
                 markersize=5,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Cluster load est at " + str(p))

  if options.wantReleases :
    for p in workers :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and release[i] > 0:
           x.append(time[i])
           y.append(release[i])
           if release[i] > ymax :
             ymax = release[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'p',color=colors[p % ncolors],
                 markersize=4,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="SP Releases at " + str(p))

  if not spPlotted :
    ax2 = ax1

  bprocs &= procset(options.blist,procs,workers,hubs,set())
  if len(bprocs) > 0 :
    for p in bprocs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and abs(bounds[i]) < 1e75 :
           x.append(time[i])
           y.append(bounds[i])
      if len(x) > 0 :
        if spPlotted and not valPlotted :
          ax2 = ax1.twinx()
        valPlotted = True;
        ax2.plot(x,y,':',color=colors[p % ncolors],
                 label="Worker bound value at " + str(p))

  iprocs &= procset(options.ilist,procs,workers,hubs,set())
  if len(iprocs) > 0 :
    for p in iprocs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and abs(incumbents[i]) < 1e75 :
           x.append(time[i])
           y.append(incumbents[i])
      if len(x) > 0 :
        if spPlotted and not valPlotted :
          ax2 = ax1.twinx()
        valPlotted = True;
        pl.plot(x,y,'-.',color=colors[p % ncolors],
                label="Incumbent value at " + str(p))

  gprocs &= procset(options.glist,procs,workers,hubs,set())
  if len(gprocs) > 0 :
    for p in gprocs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and abs(gbounds[i]) < 1e75 :
           x.append(time[i])
           y.append(gbounds[i])
      if len(x) > 0 :
        if spPlotted and not valPlotted :
          ax2 = ax1.twinx()
        valPlotted = True;
        ax2.plot(x,y,'-s',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Global bound value at " + str(p))

  cprocs &= procset(options.clist,procs,workers,hubs,set())
  if len(cprocs) > 0 :
    for p in cprocs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and abs(cbounds[i]) < 1e75 :
           x.append(time[i])
           y.append(cbounds[i])
      if len(x) > 0 :
        valPlotted = True;
        pl.plot(x,y,'-x',color=colors[p % ncolors],
                markersize=4,markerfacecolor= colors[p % ncolors],
                markeredgecolor=colors[p % ncolors],
                label="Cluster bound value at " + str(p))

  hprocs &= procset(options.hlist,procs,workers,hubs,set())
  if len(hprocs) > 0 :
    for p in hprocs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and abs(hbounds[i]) < 1e75 :
           x.append(time[i])
           y.append(hbounds[i])
      if len(x) > 0 :
        if spPlotted and not valPlotted :
          ax2 = ax1.twinx()
        valPlotted = True;
        ax2.plot(x,y,'-H',color=colors[p % ncolors],
                 markersize=5,markerfacecolor= colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Hub bound value at " + str(p))

  sprocs &= procset(options.slist,procs,workers,hubs,set())
  if len(sprocs) > 0 :
    for p in sprocs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and abs(sbounds[i]) < 1e75 :
           x.append(time[i])
           y.append(sbounds[i])
      if len(x) > 0 :
        if spPlotted and not valPlotted :
          ax2 = ax1.twinx()
        valPlotted = True;
        ax2.plot(x,y,'--D',color=colors[p % ncolors],
                 markersize=3.5,markerfacecolor= colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Server bound value at " + str(p))

  if options.wantSolGen :
    for p in procs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and offers[i] > 0:
           x.append(time[i])
           y.append(offers[i])
           if offers[i] > ymax :
             ymax = offers[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'_',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Solutions generated at " + str(p))

  if options.wantAdmit :
    for p in procs :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and admits[i] > 0:
           x.append(time[i])
           y.append(admits[i])
           if admits[i] > ymax :
             ymax = admits[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'x',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Solutions admitted to repository at " + str(p))

  if options.wantDispatch :
    for p in procs.intersection(hubs) :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and dispatches[i] > 0:
           x.append(time[i])
           y.append(dispatches[i])
           if dispatches[i] > ymax :
             ymax = dispatches[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'v',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Dispatches from " + str(p))

  if options.wantReceive :
    for p in procs.intersection(workers) :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and receives[i] > 0:
           x.append(time[i])
           y.append(receives[i])
           if receives[i] > ymax :
             ymax = receives[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'d',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Subproblem receives at " + str(p))

  if options.wantRebal :
    for p in procs.intersection(workers) :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and rebals[i] > 0:
           x.append(time[i])
           y.append(rebals[i])
           if rebals[i] > ymax :
             ymax = rebals[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'^',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Subproblems rebalanced from " + str(p))

  if options.wantLB :
    for p in procs.intersection(hubs) :
      x = []
      y = []
      for i in range(records) :
        if processor[i]==p and time[i]>=startseconds \
           and time[i]<=endseconds and lbrounds[i] > 0:
           x.append(time[i])
           y.append(lbrounds[i])
           if lbrounds[i] > ymax :
             ymax = lbrounds[i]
      if len(x) > 0 :
        spPlotted = True;
        ax1.plot(x,y,'H',color=colors[p % ncolors],
                 markersize=3,markerfacecolor=colors[p % ncolors],
                 markeredgecolor=colors[p % ncolors],
                 label="Number of load balancing rounds " + str(p))

  if not (spPlotted or valPlotted) :
    print "Warning: no valid data to plot; exiting"
    exit(0)

  ax1.set_xlim(startseconds,endseconds)
  ax1.set_ylim(0,1.05*ymax)

  if options.wantLegend :
    handles, labels = ax1.get_legend_handles_labels()
    if spPlotted and valPlotted :
      ax2.set_xlim(startseconds,endseconds)
      handles2, labels2 = ax2.get_legend_handles_labels()
      handles += handles2
      labels  += labels2
    ax1.legend(handles, labels,
               prop=mpl.font_manager.FontProperties(size='x-small'),
               loc=0)

  ax1.set_xlabel("Time (Seconds)",fontsize='small')
  if spPlotted :
    ax1.set_ylabel("Subproblems",fontsize='small')
  if valPlotted :
    ax2.set_ylabel("Objective Values",fontsize='small')

  fontsize = 10
  for tick in ax1.xaxis.get_major_ticks():
    tick.label1.set_fontsize(fontsize)
  for tick in ax1.yaxis.get_major_ticks():
    tick.label1.set_fontsize(fontsize)
  if spPlotted and valPlotted :
    for tick in ax2.yaxis.get_major_ticks():
      tick.label1.set_fontsize(fontsize)

  if options.wantOutput :
    outfile = problemname + options.suffix + "." + options.outputType
    try :
      pl.savefig(outfile,orientation='landscape',papertype='letter')
      print "   Wrote graphics file",outfile
    except :
      print "Error: could not write file",outfile
      exit(1)
    if options.launch :
      import subprocess
      launchCommand = [options.viewer,outfile]
      subprocess.Popen(launchCommand)

  if options.wantScreen :
    pl.show()
