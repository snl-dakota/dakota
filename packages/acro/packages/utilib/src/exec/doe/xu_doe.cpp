/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

//
// Adapted from C source code for 
// Xu, H. (2002). An algorithm for constructing orthogonal and nearly-orthogonal
//   arrays with mixed levels and small runs.  Technometrics, 44, No 4, 356-368.
//
// Author: Hongquan Xu (hqxu@stat.ucla.edu)
//

#include <utilib_config.h>
#include <utilib/comments.h>
#include <utilib/OptionParser.h>
#include <utilib/CharString.h>
#include <utilib/AnyRNG.h>
#include <utilib/_math.h>
#include <utilib/PM_LCG.h>

//#define MYD(x) x
#define MYD(x) 

using namespace utilib;
using namespace std;


namespace {

int nLoop=2;
int nTry=100;


class Factor
{
public:

  Factor() :
	nlevels(0),
	type('s'),
	rlower(0.0),
	rupper(0.0)
	{}

  unsigned int nlevels;
  char type;
  double rlower;
  double rupper;
  double weight;
};


/* woa  parameters: OA'(m, s_1 ...s_n, 2)
int x[n*m];   n factors, m runs , by column.
int y[n*m];   used bu MaxOA() to save the best x[][] 
int xk[m];    temp 1 factor, m runs 
int c[m*m];   coincidence matrix, c[i][i] is unused
int s[n];     levels 
int w[n];     weights
int *nLoop, *nTry, *nTry2;  iteration status
*/


/**
 * Return a balanced column with s levels
 */
void RandomOA1(BasicArray<int>& x, unsigned int s, AnyRNG& rng)
{
//
// Compute the number of complete sets that will go into the column
//
unsigned int num = x.size()/s;
MYD( cerr << "NUM " << num << endl;)
//
// Setup the complete sets
//
int k=0;
for (unsigned int i=0; i<num; i++)
  for (unsigned int j=0; j<s; j++)
    x[k++] = j;
//
// Fill in the rest of the array with randomly selected value
//
BitArray bits(s);
bits.reset();
for (unsigned int i=0; i<(x.size()-s*num); i++)
  bits.set(i);
shuffle(bits,&rng);
for (unsigned int i=0; i<s; i++) {
  if (bits(i))
     x[k++] = i;
  }
//
// Shuffle the array
//
shuffle(x,&rng);
MYD( cerr << "X " << x << endl;)
}


/**
 * Lower bound of J2, achieved by OA
 *
 * OABound=[(sum t[i])^2 + sum (s[i]-1)*(t[i])^2 - m*(sum w[i])^2]/2,
 *  where t[i]=m*w[i]/s[i]
 *
 * w[i] = s[i] = factors[i].nlevels;
 * N = nruns
 * 
 */
void Bound(BasicArray<int>& bound, const BasicArray<Factor>& factors,
							unsigned int nruns)
{
long int sumst = 0;
long int sumst2 = 0;
long int sumw  = 0;
long int sumt  = 0;
const long int t = nruns;
for (size_t i=0; i<factors.size(); i++) {
  const int m = nruns;
  sumt += t;
  sumw += factors[i].nlevels;
  sumst += (factors[i].nlevels-1)*t;
  sumst2 += (factors[i].nlevels-1)*t*t;
  long int oabound = (sumt*sumt + sumst2 - m*sumw*sumw)/2;
  long int ssdbound = (sumt*sumt + sumst*sumst/(m-1) - m*sumw*sumw)/2;

  MYD(cerr << i << " " << oabound << " " << ssdbound << endl;)
  #if 0
  bound[i] = max(
	t*(t*factors.size()*(factors.size()+1)/2 
				+ t*sumst - sumw*sumw)/2,
	t*(t*factors.size()*(factors.size()+1)/2 
				+ t*sumst*sumst/(t-1) - sumw*sumw)/2
		);
  #endif

  bound[i] = max(oabound,ssdbound);
  MYD( cerr << "i sumst sumw bound " << i << " " << sumst << " " << sumw << " " << bound[i] << endl; )
  }
}


//
// J2 =  sum_{i > j} c[i][j]^2
//
// using x[n][m] to calculate c and J2
//     note that c[i][i] is unused
//
int CalcC(BasicArray<BasicArray<int> >& x,
	   BasicArray<BasicArray<int> >& c,
	   BasicArray<Factor>& w,
	   unsigned int n)
{
const size_t nruns = x.size();
int J2 = 0;

MYD(
  for (size_t k=0; k<n; k++)
    cerr << "w[k].nlevels: " << k << " " << w[k].nlevels << endl;
)
for (size_t i=0; i<nruns; i++) {
  for (size_t j=0; j<i; j++) {
    c[i][j] = 0;
    for (size_t k=0; k<n; k++) {
      //MYD(cerr << i << " " << j << " " << k << " " << x[i][k] << " " << x[j][k] << endl;)
      if (x[i][k] == x[j][k])
         c[i][j] += w[k].nlevels;
      }
    c[j][i] = c[i][j];
    J2 += c[i][j]*c[i][j]; 
    }
  }
MYD(
     for (unsigned int i=0; i<c.size(); i++) {
       for (unsigned int j=0; j<c[0].size(); j++)
	 cerr << c[i][j] << " ";
       cerr << endl;
       }
)

return J2;
}


//
// k: current column, sk: level, bound: max(OAbound, SSDbound) */
//
// for each randomly selected balanced column, exchange rows in the column
//   until no improvement.  If it is orthogonal to others, break; else repeat
//   nTry times and save the best to x[k][] and update c[][] */
//
int AddOAColumn(BasicArray<BasicArray<int> >& x,
		 BasicArray<BasicArray<int> >& c,
		 BasicArray<int>& xk,
		 int sk,
		 int k,
       		 int bound,
		 int J2Prev,
		 int nTry,
		 AnyRNG& rng)
{
const unsigned int nruns = x.size();
if (nTry < 1) nTry=1;

int J2Best=0;

for (int ii=0; ii <nTry; ii++) {
  //
  // Generate random column
  //
  MYD( cerr << "THERE" << endl; )
  RandomOA1(xk, sk, rng);
  //
  // Update J2
  //
  int J2 = J2Prev + nruns*sk*sk*(nruns/sk-1)/2;
  MYD( cerr << "J2 " << J2 << " " << J2Prev << " " << nruns*sk*sk*(nruns/sk-1)/2 << " " << nruns << " " << sk << endl; )
  for (unsigned int i=0; i<nruns; i++) {
    for (unsigned int j=0; j<i; j++)
      if (xk[i] == xk[j])
         J2 += 2*c[i][j]*sk;
    }
  MYD( cerr << "THERE " << J2 << " " << bound << endl; )
  //
  // Iterate until no improvement
  // 
  if (J2 > bound) {
     MYD( cerr << "THERE1" << endl; )
     int SdifBest;
     do {
        //
	// find a pair (a,b) which reduces J2 most
	//
	SdifBest =0;
	int swapa=0;
	int swapb=0;  
	for (unsigned int a=1; a<nruns; a++) {
	  for (unsigned int b=0; b<a; b++) {
	    if (xk[a] == xk[b] ) continue;
	    int Sdif = 0;
	    for (unsigned int j=0; j<nruns; j++){
	      if ((j==a) || (j==b)) continue;
	      if (xk[j] == xk[a]) 
                 Sdif += c[a][j] - c[b][j];
	      else if (xk[j] == xk[b]) 
		 Sdif += c[b][j] - c[a][j];
	      }
	    if (Sdif > SdifBest) {
	       SdifBest = Sdif;
	       swapa = a;
	       swapb = b;
	       }
	    }
	  }
     MYD( cerr << "THERE1" << endl; )
        //	
	//  swap xk[a] and xk[b], and update  J2
        //
	if (SdifBest > 0) {
	   int a = swapa;
	   int b = swapb;
	   swapa = xk[a];
	   xk[a] = xk[b];
	   xk[b] = swapa;
	   J2 -= 2*SdifBest*sk;
	   }	
        } while((J2 > bound) && (SdifBest > 0));
     MYD( cerr << "THERE1" << endl; )
     }
 MYD( cerr << "THERE" << endl; )
 //     
 // Save the best result to x[k][]
 //
 if ((J2 < J2Best) || (ii == 0)){
    MYD( cerr << "J2Best update: " << J2Best << " " << J2 << " " << k << endl; )
    J2Best = J2;
    for (unsigned int i=0; i<nruns; i++)
      x[i][k] = xk[i];
    MYD( cerr << "Update: " << nruns << " " << xk.size() << endl; )
    }
 if (J2Best <= bound)
    break; // find an best column
 MYD( cerr << "THERE" << endl; )
 }
//
// Update c[m][m] by x[k][]
//
MYD( cerr << "THERE" << endl; )
for (unsigned int i=0; i<nruns; i++){
  for (unsigned int j=0; j<i; j++)
    if (x[i][k] == x[j][k])
       c[i][j]= c[j][i] += sk;
  }
MYD( cerr << "THERE " << J2Best << endl; )
//
// Return new solution value
//
return J2Best;
}


void woa(ostream& os,
	 BasicArray<Factor>& factors,
 	 unsigned int nruns,
	 BasicArray<BasicArray<int> >& xbest,
	 AnyRNG& rng)
{
//
// Setup xk, x, xbest and c
//
// x - The current solution
// c[i][j] - delta_i_j
//
BasicArray<BasicArray<int> > x;
BasicArray<BasicArray<int> > c;
BasicArray<int> xk(nruns);
c.resize(nruns);
x.resize(nruns);
xbest.resize(nruns);
for (unsigned int i=0; i<x.size(); i++) {
  x[i].resize(factors.size());
  x[i] << -99;
  xbest[i].resize(factors.size());
  }
for (unsigned int i=0; i<nruns; i++)
  c[i].resize(nruns);
const int n0=2;
//
// Setup first two columns
//
for (unsigned int i=0; i<nruns; i++) {
  x[i][0] = (i / (nruns/factors[0].nlevels) ) % factors[0].nlevels;
  }
for (unsigned int i=0; i<nruns; i++)
  x[i][1] = i % factors[1].nlevels;
//
// compute the lower bounds
//
BasicArray<int> bound(factors.size());
Bound(bound,factors,nruns);

int J2Best=-1;
int nOABest;

for (int loop=0; loop <nLoop; loop++) { 
  //
  // initial the coincidence matrix c[][] */
  //
  int J2 = CalcC(x, c, factors, n0);
  //
  // Initialize number of orthogonal columns
  //
  int nOACol = 0;
  if (J2 == bound[n0-1])
     nOACol = n0;
  //  
  // Sequentially add columns by minizing J2
  //
  for (size_t i=n0; i<factors.size(); i++) {
    //
    // Add a column
    //
    MYD( cerr << "HERE " << J2 << endl; )
    J2 = AddOAColumn(x, c, xk, factors[i].nlevels, i, bound[i], J2, nTry, rng);
    MYD( cerr << "HERE " << J2 << endl; )
    //
    // Increase nOACol if add an OA column.
    //
    if ( J2 == bound[i] )
       ++nOACol;
    }
  //
  // Save current solution:
  //   1. First time
  //   2. J2 is less than the best J2 seen so far.
  //   3. 
  //     or if there is a smaller J2 or if more OA columns,  */
  if ((loop==0) || (J2 < J2Best)) {
     J2Best = J2;
     xbest << x;
     nOABest=nOACol;
     }
  //
  // Stop if we have found a 'best' design
  //
  if (J2Best <= bound[factors.size()-1])
     break;
  }

MYD( cerr << "Bound=" << bound << " K2=" << J2Best << " nOACol=" << nOABest << 
		" loop=" << nLoop << " Try=" << nTry <<
		" Try2=" << -1 << " time=" << -1 << endl;
     for (unsigned int i=0; i<c.size(); i++) {
       for (unsigned int j=0; j<c.size(); j++)
	 cerr << c[i][j] << " ";
       cerr << endl;
       }

)
os << "# J2=          " << J2Best << endl;
os << "# Num OA Cols= " << nOABest << endl;
os << "#" << endl;
}

} // namespace


#if 0
int main(int argc, char*argv[])
{
  int  i=0, j, k, n0=0, n=0, m=0,  nLoop, nTry2, nTry ;
  int  *x, *c, *s,*w, *xk, *y, *oabound, *ssdbound, J2;
  double dfTime = (double) time(NULL);

  /* initiate parameters (n, m, s[]) */
  if (argc == 2) { 
    FILE* file;
    char line[200];
    s = CallocMem(1000, sizeof(int));
    file = fopen (argv[1], "r");
    if (!file) {
      printf ("unable to open specified file - exiting\n");
      exit(1);
      }
     
    fgets (line, 199, file);   
    sscanf (line, "%ld", &m);
  
    fgets (line, 199, file);
    sscanf (line, "%ld", &n);
    
    i = 0;
    while (fgets(line, 199, file)) {
      sscanf (line, "%ld", &s[i++]);
    }   
  }    
  else if (argc == 1) {
   printf("\nwoa: Sequentially adding (nearly) orthogonal columns"
     "\nUsage:"
     "\nwoa filename"
	   "\nwoa runs factors [s1 [s2 ...] ]"
	   "\nwoa runs factors  Try  [s1 [s2 ...] ]"
	   "\n  Try (default 100) > 11"
	  "\nAuthor: Hongquan Xu  (hqxu@umich.edu)  March 2000\n");
    return (-1);
  }
 
  /* process the command line parameters */
if (argc > 2) {  
   m = atol(argv[1]);  
   n = atol(argv[2]);  
   s = CallocMem(n, sizeof(int));        
/*    nLoop = atol(argv[3]); */
  }
  nLoop = 2;


  if (n<2 || m<2) MsgExit("n and m must be >= 2");
  x = CallocMem(n*m, sizeof(int));
  y = CallocMem(n*m, sizeof(int));
  xk = CallocMem(m, sizeof(int));
  oabound = CallocMem(n, sizeof(int));
  ssdbound = CallocMem(n, sizeof(int));
  c = CallocMem(m*m, sizeof(int)); 
  nTry2 = nTry= 100;
  
  if(argc > 3){
    if(atol(argv[3]) > 11){
      nTry2 = nTry = atol(argv[3]);  
      for(i=0; i<n && i<argc-4; i++) s[i] = atol(argv[i+4]);
    }
    else {
      for(i=0; i<n && i<argc-3; i++) s[i] = atol(argv[i+3]);
    }
  }

  if(i==0) { /* no levels specfied */
    s[0]=2;  /* default level */
    i=1;
  }
  for( ; i<n; i++) s[i] = s[i-1]; 
  if(nLoop < 0) { nLoop=-nLoop;  nTry2=0; }  /* oa only */
 
  /* call woa, with w[i]=s[i] */
  woa(x,s,s,oabound, ssdbound, c, xk,y, &J2, &n0, &n, &m, &nLoop, &nTry, &nTry2,  &dfTime);

  printf("\n#Runs=%ld, #levels=%ld,  levels=", m, n);
  for (i=0; i<n; i++) printf(" %ld", s[i]);

  printf("\nBound=%ld, J2=%ld, nOACol=%ld, loop=%ld, Try=%ld, Try2=%ld, time=%.0lfs\n", 
	 max(oabound[n-1], ssdbound[n-1]), J2, n0, nLoop, nTry, nTry2, dfTime);
  PrintX(x, n, m);
  printf ("\n");
}
#endif




void xu_doe(ostream& os, BasicArray<Factor>& factors, CharString& fname, 
			int nruns, AnyRNG& rng)
{
//
// Print the original DOE information
//
os << "#\n# DOE generated from file " << fname << "\n#\n";
//
// Call Xu's code
//
BasicArray<BasicArray<int> > x;
woa(os,factors,nruns,x,rng);
//
// Print runs
//
os << factors.size() << endl;
for (unsigned int j=0; j<factors.size(); j++) {
  os << factors[j].nlevels;
  if (factors[j].type == 'c') {
     os << " c " << factors[j].rlower << " " << factors[j].rupper;
     }
  os << endl;
  }
for (unsigned int i=0; i<x.size(); i++) {
  for (unsigned int j=0; j<x[0].size(); j++)
    os << x[i][j] << " ";
  os << endl;
  }
}




int main(int argc, char* argv[])
{
try {
//
// Process parameter values
//
utilib::OptionParser params;
params.add_usage("xu_doe [--help] <filename>");
params.parse_args(argc,argv);
   if (params.help_option())
   {
      params.write(std::cout);
      return 1;
   }

CharString filename;
filename = params.args()[1].c_str();
unsigned int nfactors=0;
BasicArray<Factor> factors;

ifstream ifstr(filename.data());
if (!ifstr)
   EXCEPTION_MNGR(runtime_error, "Bad filename \"" << filename << "\"");
ifstr >> comment_lines;
ifstr >> nfactors >> comment_lines;
factors.resize(nfactors);

CharString tmp;
for (unsigned int i=0; i<factors.size(); i++) {
  ifstr >> factors[i].nlevels;
  factors[i].weight = static_cast<double>(factors[i].nlevels);
  if (!ifstr)
     EXCEPTION_MNGR(runtime_error, "Error reading factor " << (i+1) << endl;);
  int line_ctr;
  comment_lines(ifstr, line_ctr);
  if (line_ctr == 0) {
     //
     // Read in auxillary data about this factor
     //
     ifstr.get(factors[i].type);
     ifstr >> factors[i].rlower >> factors[i].rupper >> comment_lines;
     }
  }
int nruns;
ifstr >> nruns;

PM_LCG foo;
foo.set_seed(100);
AnyRNG rng(&foo);
xu_doe(cout, factors, filename, nruns, rng);
}
catch (std::exception& err) {
cerr << err.what() << endl;
}

return 0;
}
