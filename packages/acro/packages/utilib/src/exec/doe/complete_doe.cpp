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
// Factor types:
//   c	continuous
//   s	simple
//

#include <utilib_config.h>
#include <utilib/comments.h>
#include <utilib/OptionParser.h>
#include <utilib/CharString.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif
using namespace utilib;


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
  
};

ostream& operator<<(ostream& os, const Factor& ) {return os;}
istream& operator>>(istream& is, Factor& ) {return is;}


void complete_doe_recursion(ostream& os, BasicArray<Factor>& factors, BasicArray<int>& work, unsigned int i)
{
//
// Print experimental information recursively
//
for (unsigned int j=0; j<factors[i].nlevels; j++) {
  work[i] = j;
  if ((i+1) == factors.size()) {
     for (unsigned int k=0; k<work.size(); k++)
       os << (work[k]+1) << " ";
     os << endl;
     }
  else
     complete_doe_recursion(os, factors, work, i+1);
  }
}


void complete_doe(ostream& os, BasicArray<Factor>& factors, CharString& fname)
{
//
// Print the original DOE information
//
os << "#\n# Complete DOE generated from file " << fname << "\n#\n";
os << factors.size() << endl;
for (unsigned int j=0; j<factors.size(); j++) {
  os << factors[j].nlevels;
  if (factors[j].type == 'c') {
     os << " c " << factors[j].rlower << " " << factors[j].rupper;
     }
  os << endl;
  }
//
// Compute and print the number of designs
//
int num=1; 
for (unsigned int j=0; j<factors.size(); j++)
  num *= factors[j].nlevels;
os << num << endl;
//
// Start the recursion
//
BasicArray<int> work(factors.size());
complete_doe_recursion(os,factors,work,0);
}




int main(int argc, char* argv[])
{
try {
//
// Process parameter values
//
utilib::OptionParser params;
params.add_usage("complete_doe [--help] <filename>");
params.description = "A tool for performing a simple design-of-experiments";

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

complete_doe(cout, factors, filename);
}
catch (std::exception& err) {
cerr << err.what() << endl;
}

return 0;
}
