/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */
//
//  logAnalyze.cpp
//
//  Scans validation logs produced by PEBBL to detect errors and lost
//  subproblems.
//
//  Jonathan Eckstein
//
//  Not the most elegant C++ ever written, but it's not part of the
//  main library for PEBBL.

// CAP: The proper way to modify this to do MIP-specific things is to
// derive MIP-specific data structures and then have a separate
// driver/main routine.  Since we're arguing for expediency rather
// than elegance here, I'm adding the few extra pieces of information
// to the regular subproblem piece so we can keep this code
// monolithic.  Non-mip applications just won't use them.  As long as
// we aren't running out of space, etc, we probably won't need to do
// the "right" thing.


#include <acro_config.h>
#include <utilib/_math.h>
#include <utilib/ParameterList.h>
#include <pebbl/fundamentals.h>

#if defined(NO_STRING_STREAM)
#define ISSTREAM istrstream
#define OSSTREAM ostrstream
#else
#define ISSTREAM std::istringstream
#define OSSTREAM std::ostringstream
#endif

using namespace pebbl;
using namespace utilib;
using namespace std;

int    minChildren=1;
int    maxChildren=MAXINT;
bool   testObjective=false;
double trueObjective=0.0;


// If the validation logs were from MIP, they'll have some extra information
// for creating graphical output.  Even if the graphical output if off, we
// have to parse the files.  Default is from MIP
bool fromMIP=true;

// For MIP
enum branch_type {branch_up, branch_down, no_branch};


// Create a tree.dot file for the dot (dotty) program
bool makeDot=false;
int printTreeDepth=5;


//  Globals (ugh).  I never said it was elegant.

int errorCount = 0;
int fathomRead = FALSE;
double sol     = 0;
double sense   = 1;
double relTol  = 1e-7;
double absTol  = 0;
double eqTol   = 1e-5;

int processors = 0;
int lowestPNWithSubproblems;


//  This class stores information about a subproblem.

class spRecord
{
public:

  int created;
  int packedChild;
  int packed;
  int bounded;
  int split;
  int destroyed;
  int unpacked;

  int parentPN;
  int parentSerial;

  double creationBound;
  double bound;
  double splitBound;
  double destroyBound;

  int children;
  int childrenMade;
  int childrenDestroyed;

  // start MIP-specific stuff
  int branchVariable;
  // This from milp.h
  branch_type branchType;
  double branchValue;
  int depth; // used to control tree size in output graph

  void writeName(OSSTREAM& bufferStream, int this_p, int this_serial);

  // end MIP-specific stuff

  spRecord();                 // Constructor.

  void check(int p,int s);    // Check internal consistency.
  
};


spRecord::spRecord()
{
  created     = 0;
  packedChild = 0;
  packed      = 0;
  bounded     = 0;
  split       = 0;
  destroyed   = 0;
  unpacked    = 0;

  parentPN     = -1;
  parentSerial = -1;

  children          = 0;
  childrenMade      = 0;
  childrenDestroyed = 0;

  destroyBound = -sense*MAXDOUBLE;
}


void spRecord::writeName(OSSTREAM& bufferStream, int this_p, int this_serial)
{
  bufferStream << "[";
  if (processors > 1)
    bufferStream << this_p << ", ";
  bufferStream << this_serial<< "]\\nbound = " << bound << "\\n";
  bufferStream << char(0);  //Terminate the string (especially since we reuse
                            // the buffers
}


//  More globals...

spRecord** sp = 0;
int* spCount = 0;


//  Global routines.

const char* plural(int count,const char* suffix="s")
{
  if (count == 1)
    return "";
  else
    return suffix;
}


inline void mistake(ostream& errorStream)
{
  errorCount++;
  errorStream << ".\n";
}


void mistakeSP(ostream& errorStream,int p,int s)
{
  errorStream << " for node ";
  if (processors > 1)
    errorStream << p << ':';
  mistake(errorStream << s);
};


void countCheck(int count,
		int minVal,
		int maxVal,
		const char* string,
		int p,
		int s,
		const char* suffix = "s")
{
  const char* adjective = 0;
  int diff = 0;
  if (count < minVal)
    {
      adjective = " missing ";
      diff = minVal - count;
    }
  else if (count > maxVal)
    {
      adjective = " extra ";
      diff = count - maxVal;
    }
  else
    return;
  mistakeSP(cerr << diff << adjective << string << plural(diff,suffix),p,s);
  cerr << "Count=" << count 
       << ", target=[" << minVal << ',' << maxVal << "]\n";  // DBG
  
}


int inSequence(double b1,double b2)
{
  double denom = 1;
  if ((b1 != 0) && (b2 != 0))
    denom = max(fabs(b1),fabs(b2));
  return sense*(b2 - b1)/denom >= -relTol;
}


int tooDifferent(double b1,double b2)
{
  double denom = 1;
  if ((b1 != 0) && (b2 != 0))
    denom = max(fabs(b1),fabs(b2));
  return fabs(b2 - b1)/denom > eqTol;
}


void sequenceCheck(double b1,
		   double b2,
		   const char* name1,
		   const char* name2,
		   int p,
		   int s)
{
  if (!inSequence(b1,b2))
    mistakeSP(cerr << name2 << ' ' << b2 << ' ' 
	      << (sense==1 ? '<' : '>') << ' ' << name1 << ' ' << b1,
	      p,s);
}

      
int canFathom(double bound)
{
  double absGap = (sol - bound)*sense;
  if (absGap < absTol)
    return TRUE;
  double denom = max(fabs(bound),fabs(sol));
  if (denom == 0)
    return TRUE;
  else return absGap/denom <= relTol;
}


int root(int p, int s) 
{ 
  return (p == lowestPNWithSubproblems) && (s == 1);
}


//  This base class reads through the logs and parses them.  Each
//  pass of the analysis is implemented as a different derived class.

class readLog
{
public:

  void execute(int argc,char** argv);

  virtual void check() { };

  virtual void createInfo(int /*packchild*/,
			  double /*bound*/,
			  int /*parentPN*/,
			  int /*parentSerial*/,
			  // for MIP only
			  branch_type /*branchType*/,
			  int /*depth*/) 
    { }
  virtual void packInfo() { };
  virtual void boundInfo(double /*bound*/) { };
  // The last two inputs are for MIP only
  virtual void splitInfo(int /*nChildren*/,double /*bound*/, int /*branchVariable*/,
			 double /*branchValue*/) { };
  virtual void destroyInfo(int /*nChildren*/,double /*bound*/) { };
  virtual void unpackInfo(int /*nChildren*/, branch_type /*branchType*/, int /*depth*/) { };
			   
  virtual void endOfPass() { };

  int pn;
  int serial;

};


void readLog::execute(int argc,char** argv)
{
  int fileCount = 0;
  for(int arg=1; arg<argc; arg++)
    {
      ifstream log;
      log.open(argv[arg]);
      if (!log)
	{
	  mistake(cerr << "Can't open file " << argv[arg]);
	  if (*argv[arg] == '-') 
	    cerr << "Option syntax is '--parameter=value' "
		 << "(using two hyphens).\n";
	  return;
	}
      fileCount++;
      while(!log.eof())
	{
	  char verb[256];
	  double bound, branchValue;
	  int nChildren, branchVariable;
	  int branch_tmp, depth;

#if defined(NO_STRING_STREAM)
	  char line[2048];
	  log.getline(line,255);
	  if ((line[0] == '\0') && log.eof())
	    break;
	  istrstream lineStream(line,255);
#else
	  string line;
	  getline(log, line);
	  //log.getline(line.c_str(),255);
	  if ((line[0] == '\0') && log.eof())
	    break;
	  istringstream lineStream(line);
#endif
	  lineStream >> verb;
	  if (!strcmp(verb,"fathoming"))
	    {
	      lineStream >> sol >> sense >> relTol >> absTol;
	      fathomRead = TRUE;
	    }
	  else
	    {
	      lineStream >> pn >> serial;
	      check();
	      int packchild = !strcmp(verb,"packchild");
	      if (packchild || !strcmp(verb,"create"))
		{
		  int parentPN;
		  int parentSerial;
		  lineStream >> bound >> parentPN >> parentSerial;
		      
		  // packed children will give branchtype on unpacking
		  if (fromMIP && !packchild)
		    lineStream >> branch_tmp >> depth;
		  createInfo(packchild,
			     bound,
			     parentPN,
			     parentSerial, 
			     (branch_type) branch_tmp, 
			     depth);
		}
	      else if (!strcmp(verb,"pack"))
		packInfo();
	      else if (!strcmp(verb,"bound"))
		{
		  lineStream >> bound;
		  boundInfo(bound);
		}
	      else if (!strcmp(verb,"split"))
		{
		  lineStream >> nChildren >> bound;
		  if (fromMIP)
		    lineStream >> branchVariable >> branchValue;
		  splitInfo(nChildren,bound, branchVariable, branchValue);
		}
	      else if (!strcmp(verb,"destroy"))
		{
		  lineStream >> nChildren >> bound;
		  destroyInfo(nChildren,bound);
		}
	      else if (!strcmp(verb,"unpack"))
		{
		  lineStream >> nChildren;
		  if (fromMIP)
		    lineStream >> branch_tmp >> depth;
		  unpackInfo(nChildren, (branch_type)branch_tmp, depth);
		}
	      else
		mistake(cerr << "Unrecognized event '" 
			<< verb << "' in log file " << argv[arg]);
	    }
	}
    }

  if (fileCount == 0)
    mistake(cerr << "Arguments should contain at least one "
	    << "log file or '--help'");
  else
    endOfPass();

}


//  The first pass just figures out how many processors there are,
//  and barfs at any illegal processor codes.
		  
class pass1object : public readLog
{
public:

  void check()
    {
      if (pn < 0)
	mistake(cerr << "Processor number " << pn << " is illegal");
      else if (pn >= processors)
	processors = pn + 1;
    };

  void endOfPass()
    {
      cout << processors << " processor" << plural(processors) << ".\n";
      if (processors == 0)
	mistake(cerr << "Input appears to be empty");
      else if (!fathomRead) 
	{
	  cout << "Solution value is unknown.\n";
	  mistake(cerr << "No information on solution or fathoming criteria");
	}
      else
	{
	  cout << "Solution value is " << sol << ".\n";
	  if (testObjective && tooDifferent(sol,trueObjective))
	    mistake(cerr << "Incorrect objective: obtained " << sol
		    << " instead of correct value of " << trueObjective);
	  sp = new spRecord*[processors];
	  spCount = new int[processors];
	  for (pn = 0; pn < processors; pn++)
	    spCount[pn] = 0;
	}
    };
};


//  The second pass figures out how many subproblems happened on each
//  processor.

class pass2object : public readLog
{
public:
  
  void check()
    {
      if (serial <= 0)
	mistake(cerr << "Invalid subproblem ID " << pn << ':' << serial);
      else if (serial > spCount[pn])
	spCount[pn] = serial;
      if (pn < lowestPNWithSubproblems)
	lowestPNWithSubproblems = pn;
    };

  void endOfPass()
    {
      int totalSPs = 0;
      for (pn = 0; pn < processors; pn++)
	{
	  totalSPs += spCount[pn];
	  sp[pn] = new spRecord[spCount[pn] + 1];
	  if (sp[pn] == 0)
	    {
	      mistake(cerr << "Ran out of memory at " << totalSPs 
		      << " subproblems");
	      return;
	    }
	}
      cout << totalSPs << " subproblem" << plural(totalSPs) << ".\n";
    };
};


//  Pass 3 does most of the real work.

class pass3object : public readLog
{
public:
  
  void createInfo(int packchild,
		  double bound,
		  int parentPN,
		  int parentSerial, 
		  branch_type branchType, 
		  int depth);

  void packInfo();

  void boundInfo(double bound);

  void splitInfo(int nChildren,
		 double bound, 
		 int branchVariable,
		 double branchValue);

  void destroyInfo(int nChildren,double bound);

  void unpackInfo(int nChildren, branch_type branchType, int depth);

  void endOfPass();

  int validID(int p,int s)
    {
      return (p >= 0) && (p < processors) && (s > 0) && (s <= spCount[p]);
    };

};


void pass3object::createInfo(int packchild,
			     double bound,
			     int parentPN,
			     int parentSerial,
			     branch_type branchType,
			     int depth)
{
  if (packchild)
    sp[pn][serial].packedChild++;
  else
    {
    sp[pn][serial].created++;
    sp[pn][serial].branchType = branchType;
    sp[pn][serial].depth = depth;
    }
  sp[pn][serial].creationBound = bound;
  if (!packchild && root(pn,serial))
    return;
  if (!validID(parentPN,parentSerial))
    mistakeSP(cerr << "Invalid parent " << parentPN << ':' << parentSerial,
	      pn,serial);
  else
    {
      sp[parentPN][parentSerial].childrenMade++;
      sp[pn][serial].parentPN = parentPN;
      sp[pn][serial].parentSerial = parentSerial;
    }
}


void pass3object::packInfo()
{
  sp[pn][serial].packed++;
}


void pass3object::boundInfo(double bound)
{
  sp[pn][serial].bounded++;
  sp[pn][serial].bound = bound;
}


void pass3object::splitInfo(int nChildren,double bound, int branchVariable,
			    double branchValue)
{
  sp[pn][serial].split++;
  sp[pn][serial].children = nChildren;
  sp[pn][serial].splitBound = bound;
  sp[pn][serial].branchVariable = branchVariable;
  sp[pn][serial].branchValue = branchValue;
}


void pass3object::destroyInfo(int nChildren,double bound)
{
  sp[pn][serial].destroyed++;
  sp[pn][serial].childrenDestroyed += nChildren;
  if (sense*(sp[pn][serial].destroyBound - bound) < 0)
    sp[pn][serial].destroyBound = bound;
}


void pass3object::unpackInfo(int nChildren, branch_type branchType, int depth)
{
  sp[pn][serial].unpacked++;
  sp[pn][serial].childrenDestroyed -= nChildren;
  sp[pn][serial].branchType = branchType;
  sp[pn][serial].depth = depth;
}


void pass3object::endOfPass()
{
  ofstream graphFile("tree.dot");  // open with default modes

#if defined(NO_STRING_STREAM)
  // For file output.  512 is the max size of a node label
  char *nameBuffer = new char[512];
  ostrstream nameStream(nameBuffer, 512);
  char *nameBuffer2 = new char[512];
  ostrstream nameStream2(nameBuffer2, 512);
#else
  string nameBuffer;
  ostringstream nameStream(nameBuffer);
  string nameBuffer2;
  ostringstream nameStream2(nameBuffer2);
#endif
  // To keep node labels small.  Change this if you want more precision
  nameStream.precision(5);
 
  nameStream2.precision(5);

  spRecord *parent;

  if (makeDot)
    graphFile << "digraph treeGraph {\n";
  for(pn = 0; pn < processors; pn++)
    for(serial = 1; serial <= spCount[pn]; serial++)
      {
	spRecord* p = &(sp[pn][serial]);
	p->check(pn,serial);
	if (!root(pn,serial) && 
	    (p->created + p->packedChild > 0) &&
	    validID(p->parentPN,p->parentSerial))
	  sequenceCheck(sp[p->parentPN][p->parentSerial].splitBound,
			p->creationBound,
			"parent bound",
			"Creation bound",
			pn,serial);
 // output edge from parent to this processor
	if (makeDot && !root(pn,serial) && p->depth <= printTreeDepth)
	  {
	  parent = &sp[p->parentPN][p->parentSerial];
	  nameStream.seekp(ios::beg); //reuse from the beginning
	  nameStream2.seekp(ios::beg); //reuse from the beginning
	  parent->writeName(nameStream, p->parentPN, p->parentSerial);
	  p->writeName(nameStream2, pn, serial);
	  graphFile << "\"" << nameBuffer << "\" -> \"" << nameBuffer2 << "\" [label=\"";
	  // Now Edge label, of form branchVariable <= (or >=) val
	  graphFile << "var " << parent->branchVariable;
	  if (p->branchType == branch_up)
	    graphFile << " >= " << ceil(p->branchValue) << "\"];\n";
	  else graphFile << " <= " << floor(p->branchValue) << "\"]\n;";
	  }
      }
  if (makeDot) graphFile << "size = \"12,14\";\n}\n";

}


void spRecord::check(int p,int s)
{
  // When doing a "reconfigure" restart from a checkpoint, 
  // certain subproblem ID's may be skipped completely.
  // Detect completely skipped ID's and don't check them.

  if ((created == 0) && (packedChild==0) && (packed==0) && (bounded == 0) && 
      (split==0) && (destroyed==0) && (unpacked==0))
    return;

  // Non-skipped ID -- do the checks...

  countCheck(created + packedChild,1,1,"create operation",p,s);
  if (created + packedChild < 1)
    return;

  if (!bounded)
    bound = creationBound;
  else
    sequenceCheck(creationBound,bound,"creation bound","Bound",p,s);

  if (!split)
    splitBound = bound;
  else 
    sequenceCheck(bound,splitBound,"bound","Splitting bound",p,s);
  
  if (!destroyed)
    destroyBound = splitBound;
  else
    sequenceCheck(splitBound,destroyBound,
		  "splitting bound","Destruction bound",p,s);

  countCheck(bounded,!canFathom(creationBound),1,"bound operation",p,s);

  if (bounded)
    countCheck(split,!canFathom(bound),1,"split operation",p,s);

  if (split && !canFathom(splitBound) && (children < minChildren))
    mistakeSP(cerr << "Split operation produced too few children",p,s);
  if (split && (children > maxChildren))
    mistakeSP(cerr << "Split operation produced too many children",p,s);
  countCheck(childrenDestroyed,0,canFathom(destroyBound)*children,
	     "destroyed child",p,s,"ren");
  int childrenToMake = children - childrenDestroyed;
  countCheck(childrenMade,
	     (!canFathom(splitBound))*childrenToMake,
	     childrenToMake,
	     "child",p,s,"ren");

  int target = packedChild + packed;
  countCheck(unpacked,target,target,"unpack operation",p,s);
  target += created;
  countCheck(destroyed,target,target,"destruction",p,s);
}


#ifdef ACRO_VALIDATING
void dumpArgs(char* notation,int argc,char** argv)
{
  cout << notation << ':';
  for(int i=0; i<argc; i++)
    cout << ' ' << argv[i];
  cout << endl;
}
#endif


//  This thing runs it all.

int main(int argc,char** argv)
{
  utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);

  ParameterSet params;
  params.create_parameter("minChildren",minChildren,"<int>","2",
			  "Minimum number of children a subproblem should "
			  "have if not fathomed",
			  ParameterNonnegative<int>());
  params.create_parameter("maxChildren",maxChildren,"<int>","MAXINT",
			  "Maximum children a subproblem can have",
			  ParameterNonnegative<int>());
  params.create_parameter("trueObjective",trueObjective,"<double>","0.0",
			  "Check that the objective has this value "
			  "(check omitted if not set)");
  params.create_parameter("fromMIP",fromMIP,"<bool>","true",
			  "Use additional log data specific to MIP problems");
  params.create_parameter("makeDot",makeDot,"<bool>","false",
			  "Output 'dot' file picture of search tree");
  params.create_parameter("printTreeDepth",printTreeDepth,"<int>","5",
			  "Depth of tree picture",
			  ParameterBounds<int>(0,1000));

  ParameterList plist;
  plist.register_parameters(params);
  // dumpArgs("Before process_parameters",argc,argv);
  plist.process_parameters(argc,argv,1);
  // dumpArgs("After process_parameters",argc,argv);
  params.set_parameters(plist,false);

  if (params.get_parameter<bool>("help")) 
    {
      cout << "Usage: logAnalyze {--parameter=value ...} "
	   << "logfile1 {logfile2 ...}\n";
      params.write_parameters(cout);
      return -1;
    }

  testObjective = params.parameter_initialized("trueObjective");

  pass1object pass1;
  pass1.execute(argc,argv);
  if (errorCount == 0)
    {
      lowestPNWithSubproblems = processors;
      pass2object pass2;
      pass2.execute(argc,argv);
      if (errorCount == 0)
	{
	  pass3object pass3;
	  pass3.execute(argc,argv);
	}
    }

  if (errorCount == 0)
    cout << "No";
  else
    cout << errorCount;
  cout << " error" << plural(errorCount) << ".\n";

  return errorCount;

}

